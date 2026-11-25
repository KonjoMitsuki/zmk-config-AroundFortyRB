"use strict";

async function readAnalyticsPort(port, validate, onProgress, timeoutMs = 30_000) {
  const encoder = new TextEncoder();
  const decoder = new TextDecoder("utf-8", { fatal: true });
  let pending = "";
  let reader;
  let writer;
  await port.open({ baudRate: 115200 });

  async function nextDocument(skipSyncReply) {
    for (;;) {
      const lineEnd = pending.indexOf("\n");
      if (lineEnd < 0) {
        if (pending.length > 65_536) throw new Error("応答が大きすぎます。");
        const { value, done } = await reader.read();
        if (done) throw new Error("ポートとの接続が切れました。");
        pending += decoder.decode(value, { stream: true });
        continue;
      }
      if (lineEnd > 65_536) throw new Error("応答が大きすぎます。");
      const line = pending.slice(0, lineEnd).trim();
      pending = pending.slice(lineEnd + 1);
      const doc = JSON.parse(line);
      if (skipSyncReply && doc?.error === "unknown_command") {
        skipSyncReply = false;
        continue;
      }
      if (doc?.error) throw new Error("キーボードからエラーが返されました（" + doc.error + "）。");
      return doc;
    }
  }

  async function request(command, kind, synchronize = false) {
    onProgress(kind);
    let timer;
    const timeout = new Promise((_, reject) => {
      timer = setTimeout(() => reject(new Error(command + " の応答がありません。")), timeoutMs);
    });
    try {
      const response = writer.write(encoder.encode((synchronize ? "\n" : "") + command + "\n"))
        .then(() => nextDocument(synchronize));
      const doc = await Promise.race([response, timeout]);
      if (validate(doc) !== kind) throw new Error(command + " に対して別の種類の応答が返されました。");
      return doc;
    } finally {
      clearTimeout(timer);
    }
  }

  try {
    await port.setSignals({ dataTerminalReady: true });
    await new Promise((resolve) => setTimeout(resolve, 400));
    reader = port.readable.getReader();
    writer = port.writable.getWriter();
    const snapshot = await request("GET", "snapshot", true);
    if (snapshot.version < 3) return { snapshot };
    const bigrams = await request("BIGRAMS", "bigrams");
    const recent = await request("RECENT", "recent");
    if (bigrams.positions !== snapshot.presses.length || recent.positions !== snapshot.presses.length) {
      throw new Error("取得した応答の位置数が一致しません。");
    }
    return { snapshot, bigrams, recent };
  } finally {
    reader?.releaseLock();
    writer?.releaseLock();
    await port.close();
  }
}

"use strict";

const map = AROUNDFORTY_KEYMAP_DATA;
const layoutId = "aroundforty-rb:matrix-v1";
const names = { snapshot: "打鍵集計", layers: "レイヤー集計", bigrams: "キー遷移", recent: "直近の打鍵", history: "長期履歴" };
const bounds = {
  hold: [20, 40, 60, 80, 100, 200, 500],
  interval: [50, 100, 150, 200, 300, 500, 1000],
  pair: [20, 40, 60, 80, 100],
};
const qualityLabels = {
  duplicate_downs: "重複した押下", orphan_releases: "対応する押下がない解放",
  timing_discards: "タイミングの計測除外", pair_config_errors: "ペアの設定エラー",
};
const $ = (id) => document.getElementById(id);
const fmt = (value) => value.toLocaleString("ja-JP");
const sum = (values) => values.reduce((total, value) => total + value, 0);
const percent = (part, total) => total ? (100 * part / total).toFixed(1) + "%" : "—";
const count = (value, max = 0xffffffff) => Number.isInteger(value) && value >= 0 && value <= max;
const object = (value) => value && typeof value === "object" && !Array.isArray(value);
const bool = (value) => typeof value === "boolean";
const list = (value, length, valid) => Array.isArray(value) && value.length === length && value.every(valid);
const counters = (value, length, max) => list(value, length, (n) => count(n, max));
const matrix = (value, height, width) => list(value, height, (row) => counters(row, width, 0xffff));
const matches = (value, expected) => list(value, expected.length, (n, i) => n === expected[i]);
const active = {};
let demo = false;
let selection = "";
let importQueue = Promise.resolve();
let importEpoch = 0;
let selectedPort;
let readingUsb = false;
let usbArchive;

function requireData(condition, field) {
  if (!condition) throw new Error(field + " の形式が正しくありません。");
}

const validLayerId = (id) => id === null || count(id, 254);
const layerLabel = (id) => id == null ? "不明" : "L" + id + (map.layer_names?.[id] ? " · " + map.layer_names[id] : "");

function validatePositionCounts(positionCounts, max) {
  requireData(object(positionCounts), "レイヤー別打鍵集計");
  for (const [position, layers] of Object.entries(positionCounts)) {
    requireData(/^(0|[1-9][0-9]*)$/.test(position) && count(Number(position), map.position_count - 1)
      && object(layers), "レイヤー集計の位置");
    for (const [layer, value] of Object.entries(layers)) {
      requireData(/^L(0|[1-9][0-9]*)$/.test(layer) && count(Number(layer.slice(1)), 254)
        && (value === null || count(value, max)), "レイヤー ID または打鍵数");
    }
  }
}

function validate(doc) {
  if (doc?.format === "aroundforty-analytics-history") {
    const isTime = (value) => typeof value === "string" && Number.isFinite(Date.parse(value));
    const n = map.position_count;
    const max = Math.floor(Number.MAX_SAFE_INTEGER / (n * n));
    requireData(doc.version === 1 && doc.physical_layout_id === layoutId
      && count(doc.captures, max) && doc.captures > 0 && count(doc.segments, max) && count(doc.gaps, max)
      && isTime(doc.first_capture) && isTime(doc.last_capture), "長期履歴の情報");
    requireData(list(doc.presses, n, (row) => counters(row, 2, max))
      && list(doc.transitions, n, (row) => counters(row, n, max)), "長期履歴の集計");
    requireData(Array.isArray(doc.events) && doc.events.length <= 50 && doc.events.every((event) =>
      Array.isArray(event) && event.length === 3 && isTime(event[0])
      && count(event[1], n - 1) && count(event[2], 1)), "長期履歴の直近データ");
    if (Object.hasOwn(doc, "position_counts")) validatePositionCounts(doc.position_counts, max);
    if (Object.hasOwn(doc, "event_layers")) {
      requireData(list(doc.event_layers, doc.events.length, validLayerId), "長期履歴の直近レイヤー");
    }
    return "history";
  }
  if (object(doc) && doc.schema_version === 2 && doc.protocol_version === 1
      && doc.device_model === "AroundForty-RB") {
    requireData(doc.identity?.physical_layout_id === layoutId, "物理配列 ID");
    validatePositionCounts(doc.position_counts);
    return "layers";
  }
  requireData(object(doc) && doc.protocol === "zmk-analytics-poc" && [1, 2, 3].includes(doc.version), "Analytics PoC JSON");
  const kind = doc.kind ?? "snapshot";
  if (kind === "bigrams" || kind === "recent") {
    const n = doc.positions;
    requireData(doc.version === 3 && count(n, 256) && n > 0, "位置数またはバージョン");
    if (kind === "bigrams") {
      requireData(bool(doc.saturated) && matrix(doc.counts, n, n), "遷移行列");
    } else {
      requireData(count(doc.capacity, 65535) && doc.capacity > 0 && count(doc.count, doc.capacity)
        && count(doc.next_sequence), "直近の打鍵の集計情報");
      let previous = 0;
      requireData(list(doc.events, doc.count, (event) => {
        if (!Array.isArray(event) || event.length !== 4) return false;
        const [sequence, timestamp, position, source] = event;
        if (!count(sequence) || sequence <= previous || !count(timestamp) || !count(position, n - 1) || !count(source, 1)) return false;
        previous = sequence;
        return true;
      }), "直近の打鍵");
      if (Object.hasOwn(doc, "layers")) {
        requireData(list(doc.layers, doc.count, validLayerId), "直近のレイヤー ID");
      }
    }
    return kind;
  }

  requireData(kind === "snapshot" && doc.measurement === "source_events"
    && doc.development_unprotected === true && bool(doc.persistent), "計測情報");
  const n = doc.presses?.length;
  requireData(count(n, 256) && n > 0 && list(doc.presses, n, (row) => counters(row, 2))
    && counters(doc.releases, 2) && count(doc.invalid_positions) && bool(doc.saturated), "打鍵カウンター");
  if (doc.version === 1) {
    requireData(!doc.persistent, "保存情報");
  } else {
    const storage = doc.storage;
    requireData(object(storage) && ["disabled", "loading", "ok", "error", "no_valid_snapshot", "incompatible_layout"].includes(storage.state)
      && count(storage.generation) && count(storage.error_count_session) && Number.isInteger(storage.last_error)
      && bool(storage.dirty) && bool(storage.recovered), "保存情報");
    if (doc.persistent) {
      requireData(typeof doc.physical_layout_id === "string" && doc.physical_layout_id.length
        && doc.layout_tracking === "manual", "物理配列 ID");
    }
  }
  if (doc.version === 3) {
    requireData(object(doc.quality) && Object.keys(qualityLabels).every((key) => count(doc.quality[key]))
      && bool(doc.quality.histogram_saturated), "計測品質");
    const t = doc.timing;
    requireData(object(t) && matches(t.hold_bins_ms, bounds.hold) && matches(t.interval_bins_ms, bounds.interval)
      && matches(t.pair_bins_ms, bounds.pair), "タイミング区間");
    for (const metric of ["hold", "same_key", "idle_before_press"]) {
      requireData(matrix(t[metric], n, 8), metric);
    }
    requireData(Array.isArray(t.pairs) && t.pairs.length <= 16 && t.pairs.every((pair) =>
      object(pair) && counters(pair.positions, 2, n - 1) && pair.positions[0] !== pair.positions[1]
      && matrix(pair.near, 3, 6) && matrix(pair.overlap, 3, 6)), "指定ペア");
  }
  return "snapshot";
}

function element(tag, text, className) {
  const node = document.createElement(tag);
  if (text !== undefined) node.textContent = text;
  if (className) node.className = className;
  return node;
}

function selectOptions(id, options) {
  const select = $(id);
  const previous = select.value;
  select.replaceChildren(...options.map(([value, label]) => new Option(label, value)));
  if (options.some(([value]) => String(value) === previous)) select.value = previous;
}

function empty(id, text) {
  $(id).replaceChildren(element("p", text, "empty"));
}

function table(id, columns, rows) {
  const wrapper = element("div", undefined, "table-wrap");
  const grid = element("table");
  const header = grid.createTHead().insertRow();
  for (const label of columns) {
    const cell = element("th", label);
    cell.scope = "col";
    header.append(cell);
  }
  const body = grid.createTBody();
  for (const values of rows) {
    const row = body.insertRow();
    for (const value of values) {
      const cell = row.insertCell();
      cell.textContent = value;
    }
  }
  wrapper.append(grid);
  $(id).replaceChildren(wrapper);
}

function documentFor(kind) {
  const history = active.history?.doc;
  if (history && kind === "snapshot") return history;
  if (history && kind === "bigrams") {
    return { counts: history.transitions, positions: history.presses.length, physical_layout_id: history.physical_layout_id };
  }
  return active[kind]?.doc;
}

function hasLayout(doc) {
  if (!doc) return true;
  const positions = doc.presses?.length ?? doc.positions;
  return positions === map.position_count && (doc.physical_layout_id == null || doc.physical_layout_id === layoutId);
}

function keyName(position, doc) {
  if (!hasLayout(doc)) return "位置 " + position;
  const key = map.keys[position];
  return key ? key[$("layout").value].tap + " / " + key.coord : "位置 " + position;
}

function positionOptions(id, all, length, doc) {
  selectOptions(id, [["", all], ...Array.from({ length }, (_, i) => [i, keyName(i, doc)])]);
}

function chooseKey(position, focusId) {
  selection = String(position);
  renderHeatmap();
  $("timing-key").value = selection;
  renderTiming();
  if (focusId) $(focusId)?.focus({ preventScroll: true });
}

function placeKey(element, key) {
  const { width, height, key_width, key_height } = map.canvas;
  element.style.left = (100 * key.x / width) + "%";
  element.style.top = (100 * key.y / height) + "%";
  element.style.width = (100 * key_width / width) + "%";
  element.style.height = (100 * key_height / height) + "%";
}

function heatColors(values) {
  const known = values.filter((value) => value != null);
  if (!known.length) return [];
  const average = sum(known) / known.length;
  const deviation = Math.sqrt(sum(known.map((value) => (value - average) ** 2)) / known.length);
  return values.map((value) => {
    if (value == null) return null;
    const z = deviation ? (value - average) / deviation : 0;
    const hue = Math.round(110 - 55 * Math.max(-2, Math.min(2, z)));
    return `hsl(${hue} 76% 66%)`;
  });
}

function heatKey(button, color) {
  if (!color) return;
  button.classList.add("heat-key");
  button.style.backgroundColor = color;
}

function renderHeatmap() {
  const doc = documentFor("snapshot");
  const entries = doc?.presses ?? [];
  const source = $("source").value;
  const values = entries.map((row) => source === "all" ? sum(row) : row[Number(source)]);
  const total = sum(values);
  const colors = heatColors(values);
  const board = $("board");
  const mapped = hasLayout(doc);
  const boardScroll = board.parentElement;
  boardScroll.hidden = !mapped;
  $("heat-legend").hidden = !doc || !mapped;
  $("layout-note").textContent = !mapped
    ? "このファイルには対応する物理配列がありません。位置番号で表示します。"
    : "キーを選ぶと詳細とタイミングを確認できます。ラベルはリポジトリのベース配列です。";
  board.style.aspectRatio = map.canvas.width + " / " + map.canvas.height;
  board.replaceChildren();
  if (mapped) {
    for (const key of map.keys) {
      const pos = key.position;
      const value = values[pos];
      const button = element("button", undefined, "key");
      button.id = "key-" + pos;
      button.type = "button";
      placeKey(button, key);
      heatKey(button, colors[pos]);
      const name = keyName(pos, doc);
      button.title = name;
      button.setAttribute("aria-label", name + (doc ? " " + fmt(value) + "回" : ""));
      button.setAttribute("aria-pressed", String(selection === String(pos)));
      button.append(element("b", key[$("layout").value].tap), element("span", doc ? fmt(value) : "—"));
      button.addEventListener("click", () => chooseKey(pos, button.id));
      board.append(button);
    }
  }

  const ranked = values.map((value, i) => ({ value, i })).filter(({ value }) => value > 0);
  ranked.sort((a, b) => b.value - a.value || a.i - b.i);
  if (!ranked.length) {
    empty("ranking", doc ? "該当する打鍵はありません。" : "打鍵集計 JSON を開くと表示します。");
  } else {
    const ranking = element("ol");
    for (const { value, i } of ranked.slice(0, 10)) {
      const item = element("li");
      const button = element("button", keyName(i, doc));
      button.type = "button";
      button.id = "rank-" + i;
      button.append(element("span", fmt(value), "count"));
      button.addEventListener("click", () => chooseKey(i, button.id));
      item.append(button);
      ranking.append(item);
    }
    $("ranking").replaceChildren(ranking);
  }
  const row = selection === "" ? null : entries[Number(selection)];
  const key = mapped && selection !== "" ? map.keys[Number(selection)] : null;
  const hold = key?.[$("layout").value].hold;
  const detail = selection === "" ? "キーが選択されていません。" : keyName(Number(selection), doc)
    + (hold ? " · ホールド " + hold : "")
    + (row ? " · 右側 " + fmt(row[0]) + "回 / 左側 " + fmt(row[1]) + "回 · 発生元の絞り込み内で " + percent(values[Number(selection)], total) : " · 集計未読込");
  $("key-detail").textContent = detail;
}

function renderLayers() {
  const history = documentFor("history");
  const recent = documentFor("recent");
  const standalone = documentFor("layers");
  let positionCounts = history?.position_counts ?? standalone?.position_counts ?? {};
  let recordedTotal = history ? sum(history.presses.map(sum)) : null;
  if (!history && !standalone && recent?.layers && hasLayout(recent)) {
    recent.events.forEach(([, , position], index) => {
      const id = recent.layers[index];
      if (id === null) return;
      const key = "L" + id;
      const counts = positionCounts[position] ??= {};
      counts[key] = (counts[key] ?? 0) + 1;
    });
    recordedTotal = recent.count;
  }
  const select = $("layer");
  const board = $("layer-board");
  const details = $("layer-detail");
  const layers = [...new Set(Object.values(positionCounts).flatMap(Object.keys))]
    .sort((a, b) => Number(a.slice(1)) - Number(b.slice(1)));
  selectOptions("layer", layers.map((id) => [id, layerLabel(Number(id.slice(1)))]));
  select.disabled = board.parentElement.hidden = details.hidden = !layers.length;
  $("layer-legend").hidden = !layers.length;
  board.replaceChildren();
  if (!layers.length) {
    $("layer-note").textContent = history || recent || standalone
      ? "読み込んだ履歴にレイヤー付きの打鍵はありません。"
      : "レイヤー付きのUSB履歴か長期履歴を開くと表示します。「サンプルを見る」でも試せます。";
    return;
  }
  const layer = select.value;
  const values = map.keys.map(({ position }) => {
    const value = positionCounts[position]?.[layer];
    return value === undefined && recordedTotal !== null ? 0 : value;
  });
  const known = values.filter((value) => value != null);
  const colors = heatColors(values);
  const recorded = sum(Object.values(positionCounts).flatMap((layers) => Object.values(layers)));
  $("layer-note").textContent = recordedTotal === null
    ? "色はこのレイヤー内の打鍵回数が基準です。— は記録なし、? は不明です。"
    : (history ? "長期履歴" : "直近の打鍵") + " · " + fmt(recordedTotal) + "件中 " + fmt(recorded)
      + "件にレイヤー情報があります。押下を受け付けた時点の割り当てを表示します。";
  details.textContent = layer + " · 記録された打鍵 " + (known.length ? fmt(sum(known)) + "回" : "なし");
  board.style.aspectRatio = map.canvas.width + " / " + map.canvas.height;
  for (const key of map.keys) {
    const value = values[key.position];
    const label = value === undefined ? "—" : value === null ? "?" : fmt(value);
    const detail = key.coord + " · " + (value === undefined ? "記録なし" : value === null ? "不明" : label + "回");
    const button = element("button", undefined, "key");
    button.type = "button";
    button.title = detail;
    button.setAttribute("aria-label", detail);
    button.append(element("b", key.coord), element("span", label));
    button.addEventListener("click", () => { details.textContent = layer + " · " + detail; });
    placeKey(button, key);
    heatKey(button, colors[key.position]);
    board.append(button);
  }
}

function histogram(id, edges, values, end) {
  const maximum = Math.max(...values);
  if (maximum === 0) {
    empty(id, "この条件で計測されたデータはありません。");
    return;
  }
  const chart = element("table", undefined, "hist");
  chart.setAttribute("aria-label", "計測間隔と件数");
  const body = chart.createTBody();
  for (const [i, value] of values.entries()) {
    const row = body.insertRow();
    const label = i === 0 ? "0–" + (edges[0] - 1)
      : i < edges.length ? edges[i - 1] + "–" + (edges[i] - 1)
      : end ? edges.at(-1) + "–" + (end - 1) : edges.at(-1) + "以上";
    const heading = element("th", label + " ms");
    heading.scope = "row";
    row.append(heading);
    const bar = element("span", undefined, "bar");
    bar.style.width = (100 * value / maximum) + "%";
    row.insertCell().append(bar);
    row.insertCell().textContent = fmt(value) + " 件";
  }
  $(id).replaceChildren(chart);
}

function renderTiming() {
  const doc = documentFor("snapshot");
  const timing = doc?.timing;
  $("timing-key").disabled = $("metric").disabled = !timing;
  $("pair-panel").hidden = !timing?.pairs.length;
  if (!timing) {
    empty("histogram", documentFor("history") ? "長期履歴には押下時間や指定ペアの計測結果が含まれません。"
      : doc ? "このバージョンの集計にタイミングは含まれません。" : "タイミングを含む打鍵集計 JSON を開くと表示します。");
    return;
  }
  const metric = $("metric").value;
  const rows = timing[metric];
  const values = selection === "" ? rows[0].map((_, i) => sum(rows.map((row) => row[i]))) : rows[Number(selection)];
  histogram("histogram", metric === "hold" ? bounds.hold : bounds.interval, values);
  if (timing.pairs.length) {
    selectOptions("pair", timing.pairs.map((pair, i) => [i, pair.positions.map((pos) => keyName(pos, doc)).join(" ↔ ")]));
    const values = timing.pairs[Number($("pair").value)][$("pair-metric").value][Number($("pair-class").value)];
    histogram("pair-histogram", bounds.pair, values, 250);
  }
}

function svgElement(tag, attributes) {
  const node = document.createElementNS("http://www.w3.org/2000/svg", tag);
  for (const [key, value] of Object.entries(attributes)) node.setAttribute(key, value);
  return node;
}

function flowPath(a, b) {
  const { key_width, key_height, width, height } = map.canvas;
  const x = a.x + key_width / 2;
  const y = a.y + key_height / 2;
  if (a === b) {
    const direction = x + 57 > width ? -1 : 1;
    return `M ${x + 13 * direction} ${y - 16} C ${x + 57 * direction} ${y - 31}, ${x + 57 * direction} ${y + 31}, ${x + 13 * direction} ${y + 16}`;
  }
  const dx = b.x - a.x;
  const dy = b.y - a.y;
  const distance = Math.hypot(dx, dy);
  const ux = dx / distance;
  const uy = dy / distance;
  const bend = Math.min(65, 15 + distance * 0.13);
  const side = Math.min(11, distance / 6);
  const start = [x + ux * 22 - uy * side, y + uy * 22 + ux * side];
  const end = [x + dx - ux * 25 - uy * side, y + dy - uy * 25 + ux * side];
  const middle = [
    Math.max(4, Math.min(width - 4, x + dx / 2 - uy * bend)),
    Math.max(4, Math.min(height - 4, y + dy / 2 + ux * bend)),
  ];
  return `M ${start.join(" ")} Q ${middle.join(" ")} ${end.join(" ")}`;
}

function renderFlow(doc, edges, from) {
  const panel = $("flow-panel");
  panel.hidden = !doc || !edges.length || !hasLayout(doc);
  if (panel.hidden) return;
  const board = $("flow-board");
  const { width, height } = map.canvas;
  board.style.aspectRatio = width + " / " + height;
  board.replaceChildren();
  for (const key of map.keys) {
    const label = element("div", undefined, "key");
    label.classList.toggle("origin", from === String(key.position));
    label.append(element("b", key[$("layout").value].tap));
    placeKey(label, key);
    board.append(label);
  }

  const svg = svgElement("svg", {
    viewBox: `0 0 ${width} ${height}`, class: "flow-lines", "aria-hidden": "true",
  });
  const marker = svgElement("marker", {
    id: "flow-arrow", viewBox: "0 0 10 10", markerWidth: 9, markerHeight: 9,
    refX: 9, refY: 5, orient: "auto", markerUnits: "userSpaceOnUse",
  });
  marker.append(svgElement("path", { d: "M0 0 L10 5 L0 10 Z", class: "flow-arrow" }));
  const defs = svgElement("defs", {});
  defs.append(marker);
  svg.append(defs);
  const maximum = doc.counts.reduce((max, row) => Math.max(max, ...row), 0);
  for (const { a, b, value } of edges.slice(0, 12).reverse()) {
    svg.append(svgElement("path", {
      d: flowPath(map.keys[a], map.keys[b]), class: "flow-edge",
      "stroke-width": 1 + 7 * value / maximum, "marker-end": "url(#flow-arrow)",
    }));
  }
  board.append(svg);
  $("flow-scale").textContent = "線の太さの基準：このファイルで最も多い遷移 " + fmt(maximum) + "回";
}

function renderBigrams() {
  const doc = documentFor("bigrams");
  $("from").disabled = !doc;
  if (!doc) {
    renderFlow(null, [], "");
    return empty("transitions", "キー遷移 JSON（--bigrams）を開くと表示します。");
  }
  const from = $("from").value;
  const edges = [];
  for (const [a, row] of doc.counts.entries()) {
    if (from !== "" && a !== Number(from)) continue;
    const total = sum(row);
    row.forEach((value, b) => { if (value > 0) edges.push({ a, b, value, total }); });
  }
  edges.sort((a, b) => b.value - a.value || a.a - b.a || a.b - b.b);
  renderFlow(doc, edges, from);
  if (!edges.length) return empty("transitions", "この条件で計測された遷移はありません。");
  table("transitions", ["遷移元", "遷移先", "回数", "遷移元に占める割合"],
    edges.slice(0, 30).map(({ a, b, value, total }) => [keyName(a, doc), keyName(b, doc), fmt(value), percent(value, total)]));
}

function renderRecent() {
  const history = documentFor("history");
  if (history) {
    if (!history.events.length) return empty("events", "保存されている打鍵はありません。");
    table("events", ["初回取得時刻", "キー", "発生元", "レイヤー"],
      history.events.map(([at, pos, source], index) =>
        [new Date(at).toLocaleString("ja-JP"), keyName(pos, history), source === 0 ? "右側" : "左側",
          layerLabel(history.event_layers?.[index])]));
    $("events").prepend(element("p", "最近取得した最大50件。時刻はPCが記録を取り込んだ時刻です。", "hint"));
    return;
  }
  const doc = documentFor("recent");
  if (!doc) return empty("events", "直近の打鍵 JSON（--recent）を開くと表示します。");
  if (!doc.events.length) return empty("events", "保存されている打鍵はありません。");
  const start = Math.max(0, doc.events.length - 50);
  table("events", ["連番", "起動からの経過（ms）", "キー", "発生元", "レイヤー"],
    doc.events.slice(start).map(([sequence, time, position, source], index) =>
      [fmt(sequence), fmt(time), keyName(position, doc), source === 0 ? "右側" : "左側",
        layerLabel(doc.layers?.[start + index])]).reverse());
  $("events").prepend(element("p", "保持 " + fmt(doc.count) + " / " + fmt(doc.capacity) + "件 · 最新の最大50件", "hint"));
}

function renderDiagnostics() {
  const history = documentFor("history");
  if (history) {
    table("diagnostics", ["項目", "値"], [
      ["記録の取得期間", [history.first_capture, history.last_capture].map((at) =>
        new Date(at).toLocaleString("ja-JP")).join(" 〜 ")],
      ["取得ファイル", fmt(history.captures) + "件"],
      ["履歴の区間", fmt(history.segments) + "区間"],
      ["区間内で打鍵を確認できない範囲", fmt(history.gaps) + "か所"],
    ]);
    return;
  }
  const doc = documentFor("snapshot");
  const transitions = documentFor("bigrams");
  const rows = [];
  if (doc) {
    rows.push(["プロトコル", "PoC v" + doc.version], ["物理配列 ID", doc.physical_layout_id ?? "未記録"],
      ["範囲外の位置", fmt(doc.invalid_positions)], ["打鍵カウンターの飽和", doc.saturated ? "あり" : "なし"]);
    if (doc.quality) {
      for (const [key, label] of Object.entries(qualityLabels)) rows.push([label, fmt(doc.quality[key])]);
      rows.push(["タイミング集計の飽和", doc.quality.histogram_saturated ? "あり" : "なし"]);
    }
    if (doc.storage) {
      const storage = doc.storage;
      const states = {
        disabled: "無効", loading: "読込中", ok: "正常", error: "エラー",
        no_valid_snapshot: "有効な保存データなし", incompatible_layout: "配列が不一致",
      };
      rows.push(["保存状態", states[storage.state]], ["保存世代", fmt(storage.generation)],
        ["未保存の変更", storage.dirty ? "あり" : "なし"], ["保存データの復元", storage.recovered ? "あり" : "なし"],
        ["今セッションの保存エラー", fmt(storage.error_count_session) + " 件（最終コード " + storage.last_error + "）"]);
    } else {
      rows.push(["保存状態", "保存情報なし"]);
    }
  }
  if (transitions) rows.push(["遷移カウンターの飽和", transitions.saturated ? "あり" : "なし"]);
  if (!rows.length) return empty("diagnostics", "打鍵集計またはキー遷移 JSON を開くと表示します。");
  table("diagnostics", ["項目", "値"], rows);
  if (doc?.saturated || doc?.quality?.histogram_saturated || transitions?.saturated) {
    $("diagnostics").prepend(element("p", "一部のカウンターが上限に達しています。該当する回数や割合は参考値です。", "hint"));
  }
}

function render() {
  const doc = documentFor("snapshot");
  const bigrams = documentFor("bigrams");
  const sources = Object.entries(active);
  $("sources").replaceChildren(...sources.map(([kind, entry]) =>
    element("span", names[kind] + " · " + entry.name, "chip")));
  if (!sources.length) $("sources").textContent = "取得済みの JSON をここにドロップ";
  $("clear").hidden = !sources.length;
  $("usb-download").hidden = !usbArchive;
  $("source").disabled = !doc;
  $("history-note").hidden = !documentFor("history");
  const right = doc ? sum(doc.presses.map((row) => row[0])) : 0;
  const left = doc ? sum(doc.presses.map((row) => row[1])) : 0;
  const summary = [["総打鍵数", fmt(right + left)], ["左側の割合", percent(left, right + left)],
    ["右側の割合", percent(right, right + left)], ["打鍵があった位置", fmt(doc?.presses.filter((row) => sum(row) > 0).length ?? 0)]];
  $("summary").replaceChildren(...summary.map(([label, value]) => {
    const tile = element("div", undefined, "stat");
    tile.append(element("span", label), element("strong", doc ? value : "—"));
    return tile;
  }));
  positionOptions("timing-key", "すべて", doc?.presses.length ?? map.position_count, doc);
  if (Number(selection) >= (doc?.presses.length ?? map.position_count)) selection = "";
  $("timing-key").value = selection;
  positionOptions("from", "すべて", bigrams?.positions ?? 0, bigrams);
  renderHeatmap();
  renderLayers();
  renderTiming();
  renderBigrams();
  renderRecent();
  renderDiagnostics();
}

function clear() {
  for (const kind of Object.keys(active)) delete active[kind];
  demo = false;
  usbArchive = undefined;
  selection = "";
  $("source").value = "all";
  $("from").value = "";
  $("pair").value = "";
  $("layer").value = "";
}

function fileDocuments(doc) {
  if (doc?.format !== "aroundforty-analytics-capture") return [[validate(doc), doc]];
  requireData(doc.version === 1 && object(doc.documents), "取得結果");
  const entries = Object.entries(doc.documents);
  requireData(Object.hasOwn(doc.documents, "snapshot") && entries.every(([kind, item]) =>
    ["snapshot", "bigrams", "recent"].includes(kind) && validate(item) === kind), "取得結果の内容");
  return entries;
}

function setDocuments(entries, name) {
  if (demo) clear();
  if (entries.some(([kind]) => kind === "history")) clear();
  else if (entries.some(([kind]) => kind !== "layers")) delete active.history;
  for (const [kind, doc] of entries) {
    if (kind === "snapshot") {
      selection = "";
      $("pair").value = "";
    }
    if (kind === "bigrams") $("from").value = "";
    if (kind === "recent" && doc.layers && hasLayout(doc)) delete active.layers;
    active[kind] = { doc, name };
  }
}

function importFiles(files) {
  if (readingUsb) return;
  const epoch = importEpoch;
  importQueue = importQueue.then(async () => {
    const errors = [];
    for (const file of files) {
      try {
        if (file.size > 10 * 1024 * 1024) throw new Error("ファイルが10 MiBを超えています。");
        const doc = JSON.parse(await file.text());
        if (epoch !== importEpoch) return;
        const entries = fileDocuments(doc);
        if (doc?.format === "aroundforty-analytics-capture") {
          delete active.bigrams;
          delete active.recent;
        }
        setDocuments(entries, file.name);
        usbArchive = undefined;
        $("usb-status").textContent = "";
      } catch (error) {
        errors.push(file.name + " — " + (error instanceof SyntaxError ? "JSON として読み込めません。" : error.message));
      }
    }
    if (epoch !== importEpoch) return;
    $("notice").textContent = errors.join("\n");
    render();
  });
}

function setUsbBusy(busy) {
  readingUsb = busy;
  for (const id of ["usb-read", "usb-select", "usb-download", "open", "sample", "clear"]) $(id).disabled = busy;
  $("usb-select").hidden = !selectedPort;
  $("usb-read").textContent = busy ? "取得中…" : selectedPort ? "USBから再取得" : "USBから取得";
}

async function fetchUsb(choosePort = false) {
  if (readingUsb) return;
  setUsbBusy(true);
  ++importEpoch;
  $("notice").textContent = "";
  const status = $("usb-status");
  status.textContent = !selectedPort || choosePort ? "Analytics用のCOMポートを選んでください。" : "ポートに接続しています…";
  try {
    const port = !selectedPort || choosePort ? await navigator.serial.requestPort() : selectedPort;
    const documents = await readAnalyticsPort(port, validate, (kind) => {
      status.textContent = names[kind] + "を取得しています…";
    });
    selectedPort = port;
    delete active.bigrams;
    delete active.recent;
    const capturedAt = new Date().toISOString();
    setDocuments(Object.entries(documents), "USB");
    usbArchive = { format: "aroundforty-analytics-capture", version: 1, capturedAt, documents };
    render();
    status.textContent = documents.snapshot.version < 3 ? "打鍵集計を取得しました。このバージョンはキー遷移と直近の打鍵に対応していません。"
      : "打鍵集計・キー遷移・直近の打鍵を取得しました。";
  } catch (error) {
    status.textContent = "";
    if (error.name !== "NotFoundError") {
      selectedPort = undefined;
      $("notice").textContent = "USB取得に失敗しました。Analytics用のポートを選び、他のアプリで使用中でないか確認してください。\n" + error.message;
    }
  } finally {
    setUsbBusy(false);
  }
}

function downloadUsb() {
  if (!usbArchive) return;
  const url = URL.createObjectURL(new Blob([JSON.stringify(usbArchive, null, 2) + "\n"], { type: "application/json" }));
  const link = element("a");
  link.href = url;
  link.download = "aroundforty-" + usbArchive.capturedAt.replace(/[:.]/g, "-") + ".json";
  link.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}

function showSample() {
  ++importEpoch;
  clear();
  demo = true;
  const n = map.position_count;
  const protocol = { protocol: "zmk-analytics-poc", version: 3 };
  const positions = Array.from({ length: n }, (_, i) => i);
  const presses = positions.map((i) => {
    const key = map.keys[i];
    const value = (key.row === 2 ? 90 : 22) + (i * 137 % 91) + (i % 7 === 0 ? 180 : 0);
    return key.hand === "R" ? [value, 0] : [0, value];
  });
  const timingRows = (scale) => presses.map((row, i) => {
    const value = Math.floor(sum(row) / scale);
    return [1, 3, 8, 13, 9, 5, 2, 1].map((weight, bin) => Math.floor(value * weight / (9 + (i + bin) % 5)));
  });
  const samples = {
    snapshot: {
      ...protocol, measurement: "source_events", development_unprotected: true, persistent: false,
      presses, releases: [sum(presses.map((row) => row[0])), sum(presses.map((row) => row[1]))],
      invalid_positions: 0, saturated: false,
      storage: { state: "disabled", generation: 0, error_count_session: 0, last_error: 0, dirty: false, recovered: false },
      quality: { duplicate_downs: 0, orphan_releases: 0, timing_discards: 0, pair_config_errors: 0, histogram_saturated: false },
      timing: {
        hold_bins_ms: bounds.hold, interval_bins_ms: bounds.interval, pair_bins_ms: bounds.pair,
        hold: timingRows(6), same_key: timingRows(9), idle_before_press: timingRows(7),
        pairs: [{ positions: [0, 1], near: [[0, 0, 0, 0, 0, 0], [4, 7, 16, 12, 7, 20], [0, 0, 0, 0, 0, 0]],
          overlap: [[0, 0, 0, 0, 0, 0], [3, 4, 5, 2, 1, 0], [0, 0, 0, 0, 0, 0]] }],
      },
    },
    layers: {
      schema_version: 2, protocol_version: 1, device_model: "AroundForty-RB",
      identity: { physical_layout_id: layoutId },
      position_counts: Object.fromEntries(positions.map((i) => [i, {
        L0: Math.floor(sum(presses[i]) * 0.75),
        ...(i % 3 === 0 ? { L6: i % 5 === 0 ? null : (i * 7) % 45 } : {}),
      }])),
    },
    bigrams: { ...protocol, kind: "bigrams", positions: n, saturated: false,
      counts: positions.map((a) => positions.map((b) => (a + b * 3) % 11 === 0 ? 1 + (a * b) % 24 : 0)) },
    recent: { ...protocol, kind: "recent", positions: n, capacity: 1024, count: 15, next_sequence: 16,
      events: positions.slice(0, 15).map((i) => [i + 1, 1500 + i * 87, i, map.keys[i].hand === "R" ? 0 : 1]) },
  };
  for (const [a, b, value] of [[10, 11, 120], [11, 10, 62], [13, 5, 85], [14, 14, 39], [34, 17, 98], [17, 16, 51]]) {
    samples.bigrams.counts[a][b] = value;
  }
  for (const doc of Object.values(samples)) active[validate(doc)] = { doc, name: "サンプル（合成データ）" };
  $("pair-class").value = "1";
  $("notice").textContent = "";
  $("usb-status").textContent = "";
  render();
}

$("open").addEventListener("click", () => $("files").click());
$("usb-read").addEventListener("click", () => fetchUsb());
$("usb-select").addEventListener("click", () => fetchUsb(true));
$("usb-download").addEventListener("click", downloadUsb);
if (!navigator.serial) {
  $("usb-read").hidden = true;
  $("usb-status").textContent = "USBからの取得にはデスクトップ版の Chrome または Edge が必要です。";
}
$("files").addEventListener("change", (event) => {
  importFiles(Array.from(event.target.files));
  event.target.value = "";
});
$("clear").addEventListener("click", () => {
  ++importEpoch;
  clear();
  $("notice").textContent = "";
  $("usb-status").textContent = "";
  render();
  $("open").focus();
});
$("sample").addEventListener("click", showSample);
$("layout").addEventListener("change", render);
$("source").addEventListener("change", renderHeatmap);
$("layer").addEventListener("change", renderLayers);
$("timing-key").addEventListener("change", (event) => {
  selection = event.target.value;
  renderHeatmap();
  renderTiming();
});
for (const id of ["metric", "pair", "pair-metric", "pair-class"]) $(id).addEventListener("change", renderTiming);
$("from").addEventListener("change", renderBigrams);

function fileDrag(event) {
  return event.dataTransfer.types.includes("Files");
}
window.addEventListener("dragover", (event) => {
  if (fileDrag(event)) {
    event.preventDefault();
    $("drop").classList.toggle("drag", $("drop").contains(event.target));
  }
});
$("drop").addEventListener("dragleave", (event) => {
  if (!$("drop").contains(event.relatedTarget)) $("drop").classList.remove("drag");
});
window.addEventListener("drop", (event) => {
  if (!fileDrag(event)) return;
  event.preventDefault();
  $("drop").classList.remove("drag");
  if ($("drop").contains(event.target)) importFiles(Array.from(event.dataTransfer.files));
});
render();

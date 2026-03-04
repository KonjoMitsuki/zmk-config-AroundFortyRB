/*
 * JIS配列（日本語配列）キーコード定義
 * Japanese (JIS) keyboard layout key definitions for ZMK
 *
 * OS側がJIS配列に設定されている場合に、意図した記号が入力されるよう
 * USキーコードからJISキーコードへの変換マクロを定義しています。
 *
 * 使用方法: キーマップファイルで #include <dt-bindings/zmk/keys_ja.h> を追加してください。
 */

#pragma once

#include <dt-bindings/zmk/keys.h>

/* ─── 数字行シフト記号 (Shifted number row symbols) ─── */
#define JP_EXCL LS(N1)    /* ! */
#define JP_DQT LS(N2)     /* " */
#define JP_HASH LS(N3)    /* # */
#define JP_DOLLAR LS(N4)  /* $ */
#define JP_PERCENT LS(N5) /* % */
#define JP_AMPS LS(N6)    /* & */
#define JP_SQT LS(N7)     /* ' */
#define JP_LPAR LS(N8)    /* ( */
#define JP_RPAR LS(N9)    /* ) */

/* ─── 記号キー (Symbol keys) ─── */
#define JP_MINUS MINUS            /* - */
#define JP_EQUAL LS(MINUS)        /* = */
#define JP_CARET EQUAL            /* ^ */
#define JP_TILDE LS(EQUAL)        /* ~ */
#define JP_AT LEFT_BRACKET        /* @ */
#define JP_GRAVE LS(LEFT_BRACKET) /* ` */
#define JP_LBKT RIGHT_BRACKET     /* [ */
#define JP_LBRC LS(RIGHT_BRACKET) /* { */
#define JP_RBKT BACKSLASH         /* ] */
#define JP_RBRC LS(BACKSLASH)     /* } */
#define JP_SEMI SEMICOLON         /* ; */
#define JP_PLUS LS(SEMICOLON)     /* + */
#define JP_COLON APOSTROPHE       /* : */
#define JP_ASTRK LS(APOSTROPHE)   /* * */
#define JP_COMMA COMMA            /* , */
#define JP_LT LS(COMMA)           /* < */
#define JP_DOT DOT                /* . */
#define JP_GT LS(DOT)             /* > */
#define JP_SLASH SLASH            /* / */
#define JP_QMARK LS(SLASH)        /* ? */

/* ─── JIS固有キー (JIS-specific keys) ─── */
#define JP_BSLH INTERNATIONAL_1      /* \ (ろキー) */
#define JP_UNDER LS(INTERNATIONAL_1) /* _ */
#define JP_YEN INTERNATIONAL_3       /* ¥ */
#define JP_PIPE LS(INTERNATIONAL_3)  /* | */

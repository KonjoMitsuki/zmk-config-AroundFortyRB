Please generate Markdown tables for the physical keyboard layout from a ZMK keymap file.

Rules:

1. Parse the keymap file and process all layers in order.
2. Convert the program grid (bindings rows) into the physical layout for a split keyboard by inserting a center column.
3. The first table row must be the finger header (example: 小 薬 中 人 人 親 中心 親 人 人 中 薬 小).
4. Symbol keys must be shown as their symbol (e.g., , . - +).
5. Behavior keys (e.g., &lt, &mt) must be written by name or variables (e.g., lt(8, MINUS), mt_a_ctrl(LCTRL, A)).
6. For hold-tap keys, use the format "**hold**,tap". The hold output must be bold.
7. Layer numbers must be inline code (e.g., `8`).
8. Use "trans" for transparent.
9. The center column should be labeled "中心".
10. Add a heading per layer (e.g., "### Win-Base (layer 0)").
11. Append a short confirmation line such as "generated" at the end.

Optional:

- Support selecting specific layers (e.g., --layers Win-Base,Mac-Base).
- Replace existing tables by default.

Output target: keymap.md
Source file: config/AroundForty-RB.keymap

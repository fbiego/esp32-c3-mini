# Batman Chronograph Watchface – 410 × 494 (Fully Clean)

This package uses a **completely clean** background derived from your approved design.

### Background contains ONLY:
- Realistic side-on Batman cowl
- Small yellow bat emblem
- Large yellow Arabic numerals
- Empty chronograph subdial circles (no hands, no numbers, no text)

### Everything else is live LVGL:
- Animated main hour / minute / second hands
- Animated subdial hands
- Day of week
- Battery percentage
- Date
- Digital time
- Weather

## Files
| File | Description |
|------|-------------|
| batman_bg_410x494.png | Fully clean 410×494 background |
| hand_hour.png | Main hour hand |
| hand_min.png | Main minute hand |
| hand_sec.png | Main second hand |
| hand_sub.png | Small subdial hand |
| batman_watchface.c / .h | Complete LVGL implementation |
| README.md | This file |

## Integration (Claude Code)
1. Copy assets to your filesystem partition (or convert to C arrays).
2. Include the header and call `batman_watchface_create()` after LVGL + display init.
3. Adjust the three `lv_obj_set_pos()` values for the subdial hands so they sit dead-centre in the empty subdial circles.
4. Replace the placeholder battery and weather strings with live data from ChronosESP32 or the AXP2101.

Chronos screen identifier: `CF_WAVESHARE_410x502`

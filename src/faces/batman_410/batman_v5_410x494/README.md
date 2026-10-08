# Batman Chronograph Watchface - v5 source (2026-08-20)

New artwork delivery: wider background (1264x1568 jpg) + 3 hand sprites
(hour/minute/second), each drawn at an arbitrary angle with a baked-in gold
pivot hub and counterweight tail (unlike v4's straight-down, hub-less hands).
No new sub-hand was delivered this round - `hand_sub.png` here is an
unmodified copy of v4's.

## Processing (see /tmp scratchpad proc_hands.py / proc_all.py at the time,
not checked into the repo - reproducible from the steps below if needed):

1. **Background**: scaled to canvas height (494px, factor S = 494/1568 =
   0.315051), keeping full vertical extent per instruction ("keep all
   vertical"). Scaled width (398px) is less than the 410px canvas, so
   pillarboxed with 6px black bars each side ("mailbox left and right edge").

2. **Hands**: for each sprite, the pivot hub was located (small enclosed
   transparent hole in the hub disc, found via flood-fill from image
   border), then the sprite was rotated about that hub so its tip points
   straight up (clock-angle 0, matching the project's "12 o'clock" pose
   convention) instead of the diagonal angle it was drawn at, then cropped
   tight around the hub+tail+shaft.

3. **Minute-hand length correction**: per instruction, the minute hand's
   pivot-to-tip length was set to exactly 521px *in the original
   (pre-face-resize) artwork scale* before the second resize step - i.e.
   the delivered minute hand (647.9px tip length) was scaled by
   521/647.9 = 0.8041 first.

4. **Final scale-to-canvas**: all three hands (hour as-delivered, minute
   already corrected to 521px, second as-delivered) were then scaled by the
   same background factor S = 0.315051, so hand proportions stay consistent
   with the resized background. Final pivot-to-tip lengths on the 410x494
   canvas: hour 136.7px, minute 164.1px, second 217.8px.

Unlike v4, these hand sprites keep their own baked-in gold hub + tail
instead of relying on a separately-drawn LVGL "cap" circle, and their pivot
point sits inside the image (not at row 0) - see batman_410.c's HAND_BASE_DEG
and per-hand pivot coordinates, which changed accordingly. The old
code-drawn cap is commented out (not deleted) in case the baked-in hub look
doesn't hold up and it's needed again.

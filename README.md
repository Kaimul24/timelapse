# Timelapse Cam

## State Machine
**BOOT**: checks setup trigger and config

**SETUP**: starts Wi-Fi and web UI for streaming preview 

**TEST_SHOT**: pauses preview, captures full quality image, saves to SD

**START_TIMELAPSE**: saves config, turns Wi-Fi off, starts capture frame sleep loop

**CAPTURE_FRAME**: captures JPEG to SD

**SLEEP**: enters deep sleep until next frame

**ERROR**: logs problem, creates error file on SD, then sleeps
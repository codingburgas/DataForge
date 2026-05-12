# Voice Input

## VOICE-01 — Mic permission
- **Preconditions**: First launch.
- **Steps**: Open voice panel, click record.
- **Expected**: OS asks for mic permission. After allow, recording starts.
- **Priority**: Medium
- **Type**: Functional

## VOICE-02 — Dictate a task title
- **Steps**: Say `Buy milk tomorrow`.
- **Expected**: Title field gets `Buy milk tomorrow`. Optional date parse fills deadline.
- **Priority**: Medium
- **Type**: Functional

## VOICE-03 — No mic
- **Preconditions**: No mic device.
- **Steps**: Click record.
- **Expected**: Toast `No microphone found`. No crash.
- **Priority**: Low
- **Type**: Negative

#include "DropDetector.h"

// Static member initialization
DropDetector* DropDetector::instance = nullptr;

// ISR implementation
void IRAM_ATTR DropDetector::handleInterrupt() {
  if (instance) {
    instance->interruptTriggered = true;
  }
}

// Create the global drop detector instance
DropDetector dropDetector(PIN_DROP_DETECTOR, DROP_TRIGGER_MODE, DROP_DEBOUNCE_MS);

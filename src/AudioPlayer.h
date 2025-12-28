#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <M5Unified.h>
#include <unit_audioplayer.hpp>
#include "config.h"

// AudioPlayer extends AudioPlayerUnit with custom methods
class AudioPlayer : public AudioPlayerUnit {
private:
  int txPin;
  int rxPin;
  bool initialized;
  
public:
  // Constructor - pins will be set via begin()
  AudioPlayer() 
    : txPin(-1),
      rxPin(-1),
      initialized(false) {
  }
  
  // Initialize the audio player using M5.getPin() for Grove Port A
  bool begin(uint32_t baudRate = AUDIO_PLAYER_BAUD_RATE) {
    // Get the actual GPIO pins for Port A from M5Unified
    // IMPORTANT: port_a_pin1 = RX, port_a_pin2 = TX (confirmed from working example)
    int8_t port_a_pin1 = M5.getPin(m5::pin_name_t::port_a_pin1);
    int8_t port_a_pin2 = M5.getPin(m5::pin_name_t::port_a_pin2);
    
    // AudioPlayerUnit::begin(serial, RX, TX) - pass in same order as getPin
    bool success = AudioPlayerUnit::begin(&Serial1, port_a_pin1, port_a_pin2);
    
    if (success) {
      setVolume(AUDIO_PLAYER_VOLUME);
      delay(100);
      initialized = true;
    }
    return success;
  }
  
  // Play drop sound effect (assumes track 1 is the drop sound)
  void playDropSound() {
    if (!initialized) return;  // Gracefully fail if hardware not available
    playAudioByIndex(DROP_SOUND_TRACK);
  }
  
  // Check if hardware is working
  bool isInitialized() const {
    return initialized;
  }
  
  // List available audio files
  void listFiles() {
    uint16_t totalFiles = getTotalAudioNumber();
    
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(1.4);
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Audio Files:");
    M5.Display.setCursor(10, 30);
    
    if (totalFiles == AUDIO_PLAYER_STATUS_ERROR) {
      M5.Display.setTextColor(RED, BLACK);
      M5.Display.println("No SD card");
      M5.Display.setCursor(10, 50);
      M5.Display.println("or error");
    } else {
      M5.Display.setTextColor(GREEN, BLACK);
      M5.Display.printf("Total: %d", totalFiles);
      M5.Display.setCursor(10, 50);
      M5.Display.setTextColor(WHITE, BLACK);
      M5.Display.println("files");
      M5.Display.setCursor(10, 70);
      M5.Display.println("(*.mp3/wav)");
    }
  }
  
  // Test mode - play sound on button press
  void testMode() {
      // Show file list first
      listFiles();
      delay(3000);  // Show for 3 seconds
      
      displayPressToPlay();
      
      int playCount = 0;
      
      while (true)
      {
          M5.update();
          
          if (M5.BtnA.wasPressed())
          {
              playDropSound();
              playCount++;
              
              // Get current file info
              uint16_t currentFile = getCurrentAudioNumber();
              uint16_t totalFiles = getTotalAudioNumber();
              
              M5.Display.fillScreen(BLACK);
              M5.Display.setTextColor(WHITE, RED);
              M5.Display.setTextSize(1.6);
              M5.Display.setCursor(10, 10);
              M5.Display.println("Audio Test");
              M5.Display.setTextSize(2);
              M5.Display.setCursor(20, 40);
              M5.Display.setTextColor(BLACK, GREEN);
              M5.Display.println("PLAYING");
              
              // Show file number
              M5.Display.setTextSize(1.5);
              M5.Display.setCursor(10, 70);
              M5.Display.setTextColor(WHITE, BLACK);
              if (currentFile != AUDIO_PLAYER_STATUS_ERROR) {
                M5.Display.printf("File: %d/%d", currentFile, totalFiles);
              } else {
                M5.Display.println("File: ?/?");
              }
              
              M5.Display.setTextSize(1.4);
              M5.Display.setCursor(10, 90);
              M5.Display.setTextColor(YELLOW, BLACK);
              M5.Display.printf("Count: %d", playCount);
              
              // Wait for playback to finish
              while (checkPlayStatus() == AUDIO_PLAYER_STATUS_PLAYING)
              {
                  M5.update();
                  if (M5.BtnA.wasPressed())
                      break;
              }
              delay(1000); // Brief delay before updating display
              
              displayPressToPlay();
          }
          
          delay(10);
      }
  }

  void displayPressToPlay()
  {
      M5.Display.fillScreen(BLACK);
      M5.Display.setTextColor(WHITE, RED);
      M5.Display.setTextSize(1.6);
      M5.Display.setCursor(10, 10);
      M5.Display.println("Audio Test");
      M5.Display.setTextColor(WHITE, BLACK);
      M5.Display.setTextSize(1.4);
      M5.Display.setCursor(10, 50);
      M5.Display.println("Press button");
      M5.Display.setCursor(10, 70);
      M5.Display.println("to play sound");
  }
};

// Global instance (like Serial, Wire, etc.)
extern AudioPlayer audioPlayer;

#endif // AUDIO_PLAYER_H

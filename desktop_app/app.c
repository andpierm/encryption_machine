#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h> // per configurare la seriale
#include <assert.h>

#define SERIAL "/dev/ttyACM0"
#define BAUD B19200

int main() {
  int serial = open(SERIAL, O_RDWR);
  if(serial) {
    assert("Errore nell'apertura della porta seriale");
  }
  
  // configuro baud rate
  struct termios options;
  if(tcgetattr(serial, &options) == -1){ // legge impostazioni del terminale seriale
    assert("Errore durante acquisizione impostazioni terminale");
  }

  if(cfsetispeed(&options, BAUD) == -1) assert("Errore durante setting input baud rate");
  if(cfsetospeed(&options, BAUD) == -1) assert("Errore durante setting output baud rate");
  if(tcsetattr(serial, 0, &options) == -1) assert("Errore nell'applicazione degli attributi per seriale"); // 0 per opzioni opzionali
  close(serial);
  return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h> // per configurare la seriale
#include <assert.h>
#include <string.h>

#define SERIAL "/dev/ttyACM0"
#define BAUD B19200
#define MAX_ATTEMPTS 3
#define MAX_LENGTH_MSG 255

uint8_t check_auth(int serial){
  uint8_t cnt = 0;
  char password[MAX_LENGTH_MSG];
  char risposta[MAX_LENGTH_MSG];
  while(cnt < MAX_ATTEMPTS){
    printf("Inserisci password:\t");
    scanf("%s", password);
    password[strlen(password)] = 0;
    ssize_t sent = write(serial, password, strlen(password)+1);
    if(sent < 0) assert("Errore scrittura seriale");
    sleep(1);
    ssize_t n = read(serial, risposta, 2);
    if(n<0) assert("Errore nella lettura");
    if(risposta[0] == 'O' && risposta[1] == 'K') return 0;
    cnt++;
  }
  return 1;
}

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
  options.c_cflag &= ~PARENB;  // Nessuna parità
  options.c_cflag &= ~CSTOPB;  // 1 bit di stop
  options.c_cflag &= ~CSIZE;   // NO configurazione della dimensione dei dati
  options.c_cflag |= CS8;      // 8 bit di dati
  if(tcsetattr(serial, TCSANOW, &options) == -1) assert("Errore nell'applicazione degli attributi per seriale"); // 0 per opzioni opzionali

  if(check_auth(serial)){
    printf("Non sei stato correttamente autenticato\nFINE\n");
    close(serial);
    return 1;
  }
  printf("Sei stato correttamente autenticato!\n");
  
  close(serial);
  return 0;
}

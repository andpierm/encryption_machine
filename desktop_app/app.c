#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <termios.h> // per configurare la seriale
#include <stdlib.h>
#include <string.h>

#define SERIAL "/dev/ttyACM0"
#define BAUD B19200
#define MAX_ATTEMPTS 3
#define MAX_LENGTH_MSG 255 // non 255, perché serve +1 byte per '\0' 

uint8_t check_auth(int serial) {
  uint8_t cnt = 0;
  char password[MAX_LENGTH_MSG];
  char risposta[MAX_LENGTH_MSG];

  while(cnt < MAX_ATTEMPTS){
    printf("Inserisci password:\t");
    scanf("%254s", password); // 254s per evitare buffer overflow
    password[strlen(password)] = 0;
    password[MAX_LENGTH_MSG-1] = 0;
    ssize_t sent = write(serial, password, strlen(password)+1);
    if(sent < 0) {
      close(serial);
      perror("Errore scrittura seriale");
      exit(EXIT_FAILURE);
    }
    usleep(500000);
    ssize_t n = read(serial, risposta, MAX_LENGTH_MSG);
    if(n<0) {
      close(serial);
      perror("Errore nella lettura");
      exit(EXIT_FAILURE);
    }
    printf("Risposta: [%s]\n", risposta);

    if(risposta[0] == 'O' && risposta[1] == 'K') return 0;
    cnt++;
  }
  return 1;
}

void write_crypt(int serial, int mode) {
  char *msg_to_crypt = malloc(4096);
  char msg[MAX_LENGTH_MSG];
  if (!msg_to_crypt) {
    close(serial);
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }

  int len = 0;
  if (mode == 1) {
    printf("\nInserisci il messaggio:\t"); fflush(stdout);
    ssize_t n = read(0, msg_to_crypt, 4096); // 0 = stdin
    if(n < 0){
      close(serial);
      perror("Errore lettura da tastiera");
      exit(EXIT_FAILURE);
    }
    msg_to_crypt[n] = '\0';
    len = strlen(msg_to_crypt);
    if(len > 0 && msg_to_crypt[len - 1] == '\n') {
      msg_to_crypt[len - 1] = '\0';
      len--;
    }
    len -= len/255; // perché scanf cerca un '\0', ma se non ci sta, va avanti nel buffer
    printf("\n\nMessaggio criptato:\n\n"); fflush(stdout);
  } else {
    printf("\nInserisci i byte esadecimali:\t"); fflush(stdout);
    char hex_input[8192] = {0};
    ssize_t r = read(0, hex_input, sizeof(hex_input)); // 0 = stdin
    if (r < 0) {
      close(serial);
      perror("Errore lettura da tastiera");
      exit(EXIT_FAILURE);
    }
    hex_input[r] = '\0';
    for (int i = 0; i < r - 1 && len < 4096; i += 2) {
      unsigned int byte;
      if (sscanf(&hex_input[i], "%2x", &byte) == 1) { // per leggere in byte direttamente senza convertire da stringa a esadecimale
	      msg_to_crypt[len++] = (char)byte;
      }
    }
    if(len > 0 && msg_to_crypt[len - 1] == '\n') {
      msg_to_crypt[len - 1] = '\0';
      len--;
    }
    len -= len/255;
    printf("\n\nMessaggio decriptato:\n\n"); fflush(stdout);
  }

  int i = 0;
  int original_len = len;
  while(len > 0){
    int chunk = len > MAX_LENGTH_MSG ? MAX_LENGTH_MSG : len;
    *msg = chunk;
    ssize_t n = write(serial, msg, 1);
    if(n < 0){
      close(serial);
      perror("Error on write on serial - probably you have entered too many bytes on option select"); // non accade mai in quanto con scanf faccio %4s
      exit(EXIT_FAILURE);
    }

    n = read(serial, msg, 1); // aspetta ack pronto a scrivere
    if(n<0) {close(serial); fprintf(stderr, "Error: ricevuti %ld byte, ma attesi %d\n", n, chunk); free(msg_to_crypt); exit(EXIT_FAILURE);}
    if(n != 1 && *msg != 'O') {close(serial);perror("Error while reading first ACK");exit(EXIT_FAILURE);}

    n = write(serial, msg_to_crypt+i, chunk);
    if(n < 0){
      close(serial);
      perror("Error on write on serial - probably you have entered too many bytes on option select");
      exit(EXIT_FAILURE);
    }
    sleep(1);
    n = read(serial, msg, chunk);
    if(n<0 || n != chunk) {close(serial); perror("Error on reading"); exit(EXIT_FAILURE);}
    if(mode == 1){
      for (int j = 0; j < chunk; j++) {
        printf("%02x", (unsigned char)msg[j]); // non uso write perché mi stampa caratteri non stampabili
        fflush(stdout);
      }
    }else{
      for(int j = 0; j < chunk; j++){
        printf("%c", msg[j]);
        fflush(stdout);
      }
    }

    n = read(serial, msg, 1);
    if(n<0) {close(serial); perror("Error on reading"); exit(EXIT_FAILURE);}
    if(n != 1 || *msg != 'A') {close(serial); perror("Error on ACK message"); exit(EXIT_FAILURE);}
    usleep(100000);
    len -= chunk;
    i += chunk;
  }

  if(original_len % MAX_LENGTH_MSG == 0 && original_len != 0){ // caso in cui volessi inviare MULTIPLO 255 byte di lunghezza ==> arduino si aspetta che ne invii altri
    *msg = 1;
    ssize_t n = write(serial, msg, 1);
    if(n < 0){
      close(serial);
      perror("Error on write on serial - finto byte non inviato");
      exit(EXIT_FAILURE);
    }
    usleep(50000);
    n = write(serial, msg, 1); // ==> invio un byte "finto" 'o'
    if(n < 0){
      close(serial);
      perror("Error on write on serial - finto byte non inviato");
      exit(EXIT_FAILURE);
    }
    usleep(50000);
    n = read(serial, msg, 2); // contiene sia byte finto 'o' cryptato + ACK
    if(n<0 || n != 2) {close(serial); perror("Error on reading - finto byte non letto correttamente"); exit(EXIT_FAILURE);}
  }

  printf("\n");
  free(msg_to_crypt);
}

void process_file(int serial, int mode) {
  char filename[512];
  printf("\nInserisci il percorso del file:\t");
  scanf("%511s", filename);

  FILE *f = fopen(filename, "rb");
  if (!f) {
    perror("Errore apertura file");
    return;
  }

  fseek(f, 0, SEEK_END);
  long filesize = ftell(f);
  fseek(f, 0, SEEK_SET);

  if (filesize <= 0) {
    printf("File vuoto o errore nella lettura dimensione file\n");
    fclose(f);
    return;
  }

  char *msg_to_crypt = malloc(filesize);
  if (!msg_to_crypt) {
      perror("malloc fallita");
      fclose(f);
      return;
  }

  size_t total_read = 0;
  while (total_read < filesize) { // perché fread potrebbe leggere meno byte di filesize
      size_t r = fread(msg_to_crypt + total_read, 1, filesize - total_read, f);
      if (r == 0) {
          perror("Errore lettura file");
          free(msg_to_crypt);
          fclose(f);
          return;
      }
      total_read += r;
  }
  fclose(f);

  int len = (int)filesize;

  char msg[MAX_LENGTH_MSG];
  int i = 0;
  int original_len = len;

  char outfilename[520];
  if (mode == 1) {
    snprintf(outfilename, sizeof(outfilename), "%s.crypt", filename);
  } else {
    snprintf(outfilename, sizeof(outfilename), "%s.decrypt", filename);
  }

  FILE *outf = fopen(outfilename, "wb");
  if (!outf) {
    perror("Errore apertura file output");
    free(msg_to_crypt);
    return;
  }

  while (len > 0) {
    int chunk = len > MAX_LENGTH_MSG ? MAX_LENGTH_MSG : len;
    *msg = chunk;

    uint8_t len_byte = chunk;
    ssize_t n = write(serial, &len_byte, 1);
    if (n < 0) {
      close(serial);
      perror("Error on write on serial - probably you have entered too many bytes on option select");
      fclose(outf);
      free(msg_to_crypt);
      exit(EXIT_FAILURE);
    }
    usleep(50000);
    n = read(serial, msg, 1); // aspetta ack pronto a scrivere
    if(n<0) {close(serial); fprintf(stderr, "Error: ricevuti %ld byte, ma attesi %d\n", n, chunk); fclose(outf); free(msg_to_crypt); exit(EXIT_FAILURE);}
    if(n != 1 || *msg != 'O') {close(serial);perror("Error while reading first ACK");fclose(outf);exit(EXIT_FAILURE);}

    n = write(serial, msg_to_crypt + i, chunk);
    if (n < 0) {
      close(serial);
      perror("Error on write on serial - probably you have entered too many bytes on option select");
      fclose(outf);
      free(msg_to_crypt);
      exit(EXIT_FAILURE);
    }

    sleep(1);
    
    int total_read = 0;
    while (total_read < chunk) {
        ssize_t n = read(serial, msg + total_read, chunk - total_read);
        if (n < 0) {
            perror("Errore durante la lettura dalla seriale");
            close(serial);
            fclose(outf);
            free(msg_to_crypt);
            exit(EXIT_FAILURE);
        }
        if (n == 0) {
            usleep(10000);
            continue;
        }
        total_read += n;
    }

    size_t w = fwrite(msg, 1, chunk, outf);
    if (w != (size_t)chunk) {
        perror("Errore scrittura file di output");
        close(serial);
        fclose(outf);
        free(msg_to_crypt);
        exit(EXIT_FAILURE);
    }

    // Poi aspetti l'ACK 'A' come fai già
    n = read(serial, msg, 1);
    if (n < 0) {
        perror("Error on reading ACK");
    } else if (n == 0) {
        fprintf(stderr, "Nessun byte ricevuto per ACK\n");
    } else {
        if (n != 1 || msg[0] != 'A') {
            fprintf(stderr, "Errore: byte ACK ricevuto diverso da 'A'\n");
        }
    }
    i += chunk;
    len -= chunk;
    printf("\nPercentuale completamento:\t%.2f%%", 100 * original_len / chunk);
    fflush(stdout);
  }

  if(original_len % MAX_LENGTH_MSG == 0 && original_len != 0){
    *msg = 1;
    ssize_t n = write(serial, msg, 1);
    if(n < 0){
      close(serial);
      perror("Error on write on serial - finto byte non inviato");
      fclose(outf);
      free(msg_to_crypt);
      exit(EXIT_FAILURE);
    }
    usleep(50000);
    n = write(serial, msg, 1);
    if(n < 0){
      close(serial);
      perror("Error on write on serial - finto byte non inviato");
      fclose(outf);
      free(msg_to_crypt);
      exit(EXIT_FAILURE);
    }
    usleep(50000);
    n = read(serial, msg, 2);
    if(n<0 || n != 2) {close(serial); perror("Error on reading - finto byte non letto correttamente"); fclose(outf); free(msg_to_crypt); exit(EXIT_FAILURE);}
  }

  printf("\n\nFile salvato in: %s\n", outfilename);
  fclose(outf);
  free(msg_to_crypt);
}


int main() {
  int serial = open(SERIAL, O_RDWR);
  if(serial == -1) {
    perror("Errore nell'apertura della porta seriale");
    exit(EXIT_FAILURE);
  }
  sleep(3); // per dare tempo ad arduino di caricare il suo programma
  
  // configuro baud rate
  struct termios options;
  if(tcgetattr(serial, &options) == -1){ // legge impostazioni del terminale seriale
    close(serial);
    perror("Errore durante acquisizione impostazioni terminale");
    exit(EXIT_FAILURE);
  }

  if(cfsetispeed(&options, BAUD) == -1) {
    close(serial);
    perror("Errore durante setting input baud rate");
    exit(EXIT_FAILURE);
  }
  if(cfsetospeed(&options, BAUD) == -1) {
    close(serial);
    perror("Errore durante setting output baud rate");
    exit(EXIT_FAILURE);
  }
  cfmakeraw(&options);             // Imposta modalità raw - senza modificare le opzioni una ad una
  options.c_cflag |= CLOCAL | CREAD;   // Abilita ricezione

  if(tcsetattr(serial, TCSANOW, &options) == -1) {
    close(serial);
    perror("Errore nell'applicazione degli attributi per seriale"); // 0 per opzioni opzionali
    exit(EXIT_FAILURE);
  }

  if(check_auth(serial)){
    printf("Non sei stato correttamente autenticato\nFINE\n");
    close(serial);
    return 1;
  }
  printf("Sei stato correttamente autenticato!\n");

  char msg[MAX_LENGTH_MSG];
  printf("\n\n\t\t==========  OPZIONI  ==========");
  printf("\n\t<C> -- encrypt");
  printf("\n\t<D> -- decrypt");
  printf("\n\tSTOP -- terminate\n");

  while(1){
    printf("\n\nFile <F> or message <M> or <STOP>:\t");
    scanf("%4s", msg);

    if(strcmp(msg, "STOP") == 0){
      msg[strlen(msg)] = 0;
      ssize_t n = write(serial, msg, strlen(msg)+1);
      if(n<0){
        close(serial);
        perror("Error on write on serial");
        exit(EXIT_FAILURE);
      }
      break;
    }

    if(*msg == 'M'){
      printf("\nInserisci l'opzione:\t");
      scanf("%254s", msg);
      msg[strlen(msg)] = 0;
      ssize_t n = write(serial, msg, strlen(msg)+1);
      if(n<0){
        close(serial);
        perror("Error on write on serial");
        exit(EXIT_FAILURE);
      }

      if (msg[0] == 'C' && n == 2) {
	      write_crypt(serial, 1);
      } else if (msg[0] == 'D' && n == 2) {
        write_crypt(serial, 0);
      }

    }

    if(*msg == 'F'){
      printf("\nInserisci l'opzione:\t");
      scanf("%254s", msg);
      msg[strlen(msg)] = 0;
      ssize_t n = write(serial, msg, strlen(msg)+1);
      if(n < 0){
        close(serial);
        perror("Error on write on serial");
        exit(EXIT_FAILURE);
      }

      if (msg[0] == 'C' && n == 2) {
        process_file(serial, 1);
      } else if (msg[0] == 'D' && n == 2) {
        process_file(serial, 0);
      }
    }

  }
  usleep(500000); // per permettere alla seriale di inviare tutti i dati per tempo

  close(serial);
  return 0;
}

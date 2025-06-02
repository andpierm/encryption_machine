🔐 Encryption Machine

With this simple program, you can encrypt/decrypt:

    a single message (up to 4096 characters),

    or a file of any size (though... if it's huge, maybe go get some sleep 😄).

⚙️ How It Works

    Serial buffer reset:
    At startup, the desktop app flushes the Arduino’s serial buffer — just in case there’s leftover input (e.g., a stray Ctrl+C from a previous session).

    Authentication required:
    You'll be asked to enter the "insane password" that your Arduino has programmed on it.

        You have three attempts.

        If your password is longer than 255 characters, you'll lose a try.

    Choose your action:

        Encrypt/Decrypt a message

        Encrypt/Decrypt a file — the result will be saved in your current folder.

    Safe exit:
    During the encryption/decryption phase, you can terminate the program safely at any time if needed, with STOP or Ctrl+C.

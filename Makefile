# nelle prime righe vengono definite le variabili ( 'macro' ) contenenti le path
# delle cartelle necessarie per la compilazione / installazione / cleaning

SSOURCEDIR=src/server
CSOURCEDIR=src/client
SINDIR=$(SSOURCEDIR)/include
CINDIR=$(CSOURCEDIR)/include
SOBJDIR=$(SSOURCEDIR)/obj
COBJDIR=$(CSOURCEDIR)/obj
BIN=bin

# lascio all'utente la possibilità di scegliere il proprio compilatore
CC=gcc
# flag necessarie per la compilazione
FLAGS=-Wall -lpthread

# vengono definiti i file .c e .h del server e del client
_SSOURCES=chat-server.c hash.c lista.c threadMain.c threadWorker.c threadDispatcher.c utils.c userManagement.c
_SLIBS=chat-server.h hash.h lista.h threadMain.h threadWorker.h threadDispatcher.h utils.h userManagement.h common.h
_CSOURCES=chat-client.c threadListener.c threadReader.c utils.c
_CLIBS=threadListener.h threadReader.h utils.h

# stessa cosa per i file oggetto del server e del client
_SOBJ = chat-server.o hash.o lista.o threadMain.o threadWorker.o threadDispatcher.o utils.o userManagement.o
_COBJ = chat-client.o threadListener.o threadReader.o utils.o


# per come sono definiti i nomi dei files, non contengono la path completa.
# per questo motivo viene preposto ad ogni file la cartella nel quale è contenuto.
# viene utilizzata la macro patsubst
SSOURCES = $(patsubst %,$(SSOURCEDIR)/%,$(_SSOURCES))
SLIBS = $(patsubst %,$(SINDIR)/%,$(_SLIBS))
CSOURCES = $(patsubst %,$(CSOURCEDIR)/%,$(_CSOURCES))
CLIBS = $(patsubst %,$(CINDIR)/%,$(_CLIBS))

SOBJ = $(patsubst %,$(SOBJDIR)/%,$(_SOBJ))
COBJ = $(patsubst %,$(COBJDIR)/%,$(_COBJ))

# vengono compilati i files oggetto.
# il parametro '-c' viene incluso per evitare un linking precoce.
# infatti compila ed assembla ma non linka perchè è possibile che il linking non vada
# a buon fine perchè certe dipendenze non sono ancora state compilate
$(SOBJDIR)/%.o: $(SSOURCEDIR)/%.c $(SLIBS)
	@$(CC) -c -o $@ $< $(FLAGS)

$(COBJDIR)/%.o: $(CSOURCEDIR)/%.c $(CLIBS)
	@$(CC) -c -o $@ $< $(FLAGS)


# sotto ci sono i vari target
all: chat
chat: folders server client


# viene invocato 'folders' che crea le cartelle necessarie a contenere i files
folders:
	@echo "Creating directories useful for compilation"
	@mkdir -p $(SOBJDIR)
	@mkdir -p $(COBJDIR)
	@mkdir -p $(BIN)

# 'server' compila i files oggetto con le varie flags e li rende un eseguibile
server: $(SOBJ)
	@echo "Compiling Server..."
	@$(CC) -o $(SOBJDIR)/chat-server $^ $(FLAGS) $(SLIBS)
	@echo "[+] Done"

# stessa cosa per 'client'
client: $(COBJ)
	@echo "Compiling Client..."
	@$(CC) -o $(COBJDIR)/chat-client $^ $(FLAGS) $(CLIBS)
	@echo "[+] Done"

# una volta che 'make chat' ha finito, chiamando 'make install' gli eseguibili vengono
# spostati nella cartella $(BIN)
install:
	@echo "Executable files are in bin/ folder"
	@cp $(SOBJDIR)/chat-server $(BIN)/chat-server
	@cp $(COBJDIR)/chat-client $(BIN)/chat-client

# 'clean' consente di rimuovere tutti i files creati dentro al Makefile
clean:
	@echo "Cleaning files..."
	@rm -f $(SOBJDIR)/*.o
	@rm -f $(COBJDIR)/*.o
	@rm -f bin/*
	@echo "[+] Done"

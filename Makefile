#Repertoires du projet

OBJ_PERSO_DIR = bin_perso
OBJ_ENC_DIR = bin_enc
SRC_DIR = src
SRC_ENC_DIR = enc_src
INC_DIR = include

# Options de compilation 

CC = gcc
LD = gcc
INC = -I$(INC_DIR)
CFLAGS = -O3 $(INC) -Wall -Wextra -std=c99 -g
LDFLAGS = -lm -g
#-pg -fprofile-arcs

# Liste des objets realises
NEW_OBJ_FILES = $(OBJ_PERSO_DIR)/huffman.o $(OBJ_PERSO_DIR)/inout.o 
NEW_OBJ_FILES += $(OBJ_PERSO_DIR)/upsampler.o $(OBJ_PERSO_DIR)/skip_segment.o
NEW_OBJ_FILES += $(OBJ_PERSO_DIR)/idct.o $(OBJ_PERSO_DIR)/conv.o
NEW_OBJ_FILES += $(OBJ_PERSO_DIR)/iqzz.o $(OBJ_PERSO_DIR)/unpack_block.o
NEW_OBJ_FILES += $(OBJ_PERSO_DIR)/tiff.o
NEW_OBJ_FILES += $(OBJ_PERSO_DIR)/analyse_header.o 

ENC_OBJ_FILES = $(OBJ_ENC_DIR)/enc-qzz.o 
ENC_OBJ_FILES += $(OBJ_ENC_DIR)/enc-conv.o $(OBJ_ENC_DIR)/enc-tiff.o $(OBJ_ENC_DIR)/enc-downsampler.o
ENC_OBJ_FILES += $(OBJ_ENC_DIR)/enc-main.o

all : jpeg2tiff tiff2tiff

# Edition de lien des executables
jpeg2tiff : $(NEW_OBJ_FILES) $(OBJ_PERSO_DIR)/main.o
	$(LD) $(LDFLAGS) -o jpeg2tiff $(NEW_OBJ_FILES) $(OBJ_PERSO_DIR)/main.o

tiff2tiff : $(ENC_OBJ_FILES) $(NEW_OBJ_FILES)
	$(LD) $(LDFLAGS) -o tiff2tiff $(ENC_OBJ_FILES) $(NEW_OBJ_FILES)


# Compilation des sources decodeur
$(OBJ_PERSO_DIR)/main.o : $(SRC_DIR)/main.c $(INC_DIR)/jpeg.h $(INC_DIR)/inout.h $(INC_DIR)/huffman.h $(INC_DIR)/idct.h $(INC_DIR)/conv.h $(INC_DIR)/skip_segment.h $(INC_DIR)/tiff.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PERSO_DIR)/inout.o : $(SRC_DIR)/inout.c $(INC_DIR)/inout.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PERSO_DIR)/upsampler.o : $(SRC_DIR)/upsampler.c $(INC_DIR)/inout.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PERSO_DIR)/unpack_block.o : $(SRC_DIR)/unpack_block.c $(INC_DIR)/unpack_block.h $(INC_DIR)/inout.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PERSO_DIR)/analyse_header.o : $(SRC_DIR)/analyse_header.c $(INC_DIR)/analyse_header.h $(INC_DIR)/inout.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PERSO_DIR)/huffman.o : $(SRC_DIR)/huffman.c $(INC_DIR)/huffman.h $(INC_DIR)/inout.h $(INC_DIR)/jpeg.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PERSO_DIR)/idct.o : $(SRC_DIR)/idct.c $(INC_DIR)/idct.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PERSO_DIR)/conv.o : $(SRC_DIR)/conv.c $(INC_DIR)/conv.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PERSO_DIR)/iqzz.o : $(SRC_DIR)/iqzz.c $(INC_DIR)/iqzz.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PERSO_DIR)/skip_segment.o : $(SRC_DIR)/skip_segment.c $(INC_DIR)/skip_segment.h $(INC_DIR)/inout.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_PERSO_DIR)/tiff.o : $(SRC_DIR)/tiff.c $(INC_DIR)/tiff.h
	$(CC) $(CFLAGS) -o $@ -c $<

# Compilation des sources encodeur
$(OBJ_ENC_DIR)/enc-dct-vetterli.o : $(SRC_ENC_DIR)/enc-dct-vetterli.c $(INC_DIR)/enc-dct.h 
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_ENC_DIR)/enc-qzz.o : $(SRC_ENC_DIR)/enc-qzz.c $(INC_DIR)/enc-qzz.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_ENC_DIR)/enc-conv.o : $(SRC_ENC_DIR)/enc-conv.c $(INC_DIR)/enc-conv.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_ENC_DIR)/enc-tiff.o : $(SRC_ENC_DIR)/enc-tiff.c $(INC_DIR)/enc-tiff.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_ENC_DIR)/enc-downsampler.o : $(SRC_ENC_DIR)/enc-downsampler.c $(INC_DIR)/enc-downsampler.h
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_ENC_DIR)/enc-main.o : $(SRC_ENC_DIR)/enc-main.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f jpeg2tiff tiff2tiff $(NEW_OBJ_FILES) $(ENC_OBJ_FILES) $(OBJ_PERSO_DIR)/main.o
	rm -f *~ $(INC_DIR)/*~ $(SRC_DIR)/*~ $(SRC_ENC_DIR)/*~
	rm -f $(OBJ_PERSO_DIR)/*~ $(OBJ_ENC_DIR)/*~
	rm -f *.tiff

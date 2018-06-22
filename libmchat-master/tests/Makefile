.PHONY: all libmchat
LIBMCHAT_DIR = libmchat/

all: ssend srecv

libmchat:
	rm -rf $(LIBMCHAT_DIR)
	mkdir -p $(LIBMCHAT_DIR) && cd $(LIBMCHAT_DIR) && \
		cmake ../../ -DCMAKE_BUILD_TYPE=Debug
	$(MAKE) -C $(LIBMCHAT_DIR)

ssend: libmchat
	$(CC) -I../include/ simple_sender.c $(LIBMCHAT_DIR)/libmchat.a -g -lpthread -o $@

srecv: libmchat
	$(CC) -I../include/ simple_receiver.c $(LIBMCHAT_DIR)/libmchat.a -g -lpthread -o $@

peer: libmchat
	$(CC) -I../include/ -I../src/ `pkg-config --cflags --libs glib-2.0 gio-2.0` \
		-L $(LIBMCHAT_DIR) -lmchat \
		peerlist_test.c ../src/mchatv1.c ../src/mchatv1_utils.c -o $@
clean:
	rm -rf *.o $(LIBMCHAT_DIR) ssend srecv peer

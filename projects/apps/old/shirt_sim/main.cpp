/*
 * Compile me with:
 *   gcc -o tut tut.c $(pkg-config --cflags --libs gtk+-2.0 gmodule-2.0)
 */

#include <gtk/gtk.h>
#include "font8x8_basic.h"
#include <cstring>
#include <unistd.h>

int MODE = 0;
#define FLASH 2
#define ROWS 7
#define TEXTROWS 6
#define TEXTHEIGHT 7
#define TEXTWIDTH 7
#define TEXTOFFSET 2
#define WALLROWS 4
#define COLUMNS 16
#define COLORS 3

#define VERBOSE 1
#define OFF 0
#define DEBUG_LEVEL_WRITE VERBOSE
#define DEBUG_LEVEL_READ OFF
#define DEBUG_LEVEL VERBOSE

static char altframebuffer[COLUMNS][ROWS][COLORS];
static char framebufferwall[(COLUMNS * WALLROWS)][ROWS][COLORS];
static char bitmaptextbuffer[(COLUMNS * TEXTROWS)][ROWS];


char framebufferreadlock = 0;
char framebufferwritelock = 1;
GtkWidget * buff2[COLUMNS][ROWS];
int framebuffered = 0;



char* getpixelwall(int column, int row, long offset) {
	int c = column + offset;
	if(c >= (COLUMNS * WALLROWS)) {
		c = (c % (COLUMNS * WALLROWS));
		if(DEBUG_LEVEL_READ)
			printf("getpixelwall: col: %d row: %d val[0]:%d\n",c, row, framebufferwall[c][row][0]);

		if(c == 0) {
			framebuffered = 1;
		}
	}

    return framebufferwall[c][row];
}



char gettextpixel(int column, int row, long offset) {
	int c = column + offset;
	if(c >= (COLUMNS * TEXTROWS)) {
		c = c % (COLUMNS * TEXTROWS);
		if(c == 0) {
			if(framebuffered == 0) {
				framebuffered = 1;
				usleep(2000);
			}
		}
	}
	if(DEBUG_LEVEL_READ)
	printf("gettextpix: col:%d row:%d val:%d \n",c, row, bitmaptextbuffer[c][row]);
    return bitmaptextbuffer[c][row];
}
char* getpixel(int column, int row) {
    return altframebuffer[column][row];
}

void settextpixel(int column, int row, char pix) {
	if(DEBUG_LEVEL_WRITE)
	printf("settextpix - col:%d row:%d val:%d \n",column, row, pix);
	bitmaptextbuffer[column][row] = pix;
}
void setpixelwall(int column, int row, char r, char g, char b) {
	if(DEBUG_LEVEL_WRITE)
	printf("setpixelwall - col:%d row:%d val[0]:%d \n",column, row, r);
	framebufferwall[column][row][0] = r;
	framebufferwall[column][row][1] = g;
	framebufferwall[column][row][2] = b;
}
void setpixel(int column, int row, char r, char g, char b) {
	if(DEBUG_LEVEL_WRITE)
	printf("setpixel - col:%d row:%d val[0]:%d \n",column, row, r);
	altframebuffer[column][row][0] = r;
	altframebuffer[column][row][1] = g;
	altframebuffer[column][row][2] = b;
}


char bckred = 0x00;
char bckgreen = 0x00;
char bckblue = 0x00;

char bckflashred = 0xFF;
char bckflashgreen = 0x00;
char bckflashblue = 0x00;

char txtred = 0xFF;
char txtgreen = 0xFF;
char txtblue = 0x00;

void setled(int column, int row, char * data) {
	GtkWidget * lbl = buff2[column][row];
	GdkColor redd = {0, data[0], data[1], data[2]};
	if(DEBUG_LEVEL_READ)
	printf("changing pixel - col: %d - row: %d %d-%d-%d\n", column, row, data[0], data[1], data[2]);
	gtk_widget_modify_bg(lbl, GTK_STATE_NORMAL, &redd);
}

void read_wall_buffer(long *offset, int frame) {
	 for(int i = 0; i < COLUMNS; i++) {
			for(int x = 0; x < ROWS; x++) {
				char* pix = getpixelwall(i, x, *offset);
				if(frame % FLASH == 0) {
					char p[3] = { bckflashred, bckflashgreen, bckflashblue };
					setled(i, x, p);
				} else {
					setled(i, x, pix);
				}
			}
	}
}

void read_textwall_buffer(long *offset, long frame) {
	 for(int i = 0; i < COLUMNS; i++) {
			for(int x = 0; x < ROWS; x++) {
				char pix = gettextpixel(i, x, *offset);
				if(pix != 0) {
					char p[3] = { txtred, txtgreen, txtblue };
					setled(i, x, p);
				} else {
					if(frame % FLASH == 0) {
						char p[3] = { bckflashred, bckflashgreen, bckflashblue };
						setled(i, x, p);
					} else {
						char p[3] = { bckred, bckgreen, bckblue };
						setled(i, x, p);
					}

				}
			}
	}
}

void read_frame_buffer(long frame) {
	for(int i = 0; i < COLUMNS; i++) {
		for(int x = 0; x < ROWS; x++) {
			char* pix = getpixel(i, x);
			if(frame % FLASH == 0) {
				char p[3] = { bckflashred, bckflashgreen, bckflashblue };
				setled(i, x, p);
			} else {
				setled(i, x, pix);
			}
		}
	}
}


void write_wall_buffer(char * textbuffer, int len, int count) {
	int offset = 0;
	int bfstart = 0;
	for(int i = 0; i < (COLUMNS * WALLROWS); i++) {
			for(int x = 0; x < ROWS; x++) { // CREATE SOME BLINKY EFFECT
				if(i % 2 == 0) {
					if(x % 2 == 0) {
						setpixelwall(i, x, 0xFF, 0xDD, 0x33);
					} else {
						setpixelwall(i, x, 0x00, 0x11, 0x33);
					}
				} else {
					if(x % 2 == 0) {
						setpixelwall(i, x, 0x22, 0x44, 0xF3);
					} else {
						setpixelwall(i, x, 0x40, 0xFF, 0x60);
					}
				}
			}

	}
}

void write_textwall_buffer(char * textbuffer, int len) {
	int offset = 0;
	int bfstart = 0;
	for(int i = 0; i < (COLUMNS * TEXTROWS); i++) {
			if(i % TEXTHEIGHT == 0 && i != 0) {
				bfstart++;
				if(bfstart >= len) {
					bfstart = 0;
				}
				offset += 1;
			}
			for(int x = 0; x < ROWS; x++) {
				bool pix = font8x8_basic[textbuffer[bfstart]][x] & (1 << (i % ROWS));
				settextpixel(i + offset, x, pix);
			}

	}
}

void write_text_to_buffer(char* textbuffer) {
	for(int i = 0; i < COLUMNS; i++) {  // CLEAR FRAME FIRST
		for(int x = 0; x < ROWS; x++) {
			setpixel(i, x, bckred, bckgreen, bckblue);
		}
	}

	for(int i = 0; i < COLUMNS; i++) {
			for(int x = 0; x < ROWS; x++) {
				if(i < TEXTWIDTH) {
					bool pix = font8x8_basic[textbuffer[0]][x] & (1 << (i));
					if(pix) {
						setpixel(i, x, txtred, txtgreen, txtblue);
					} else {
						setpixel(i, x, bckred, bckgreen, bckblue);
					}

					int columnplace = i + TEXTWIDTH + TEXTOFFSET;

					pix = font8x8_basic[textbuffer[1]][x] & (1 << (i));
					if(pix) {
						setpixel(columnplace, x, txtred, txtgreen, txtblue);
					} else {
						setpixel(columnplace, x, bckred, bckgreen, bckblue);
					}
				}
			}
	}
}



void *react(void *ptr)
{
	MODE = 1;
	usleep(1000000);
     long offset_l = 0;
     long frame = 0;
    while(1) {

    	while(framebufferwritelock == 1) {
    		usleep(2);
    	}

		framebufferreadlock = 1;

		usleep(1000000);
		switch (MODE) {
			case 0:
				read_wall_buffer(&offset_l, frame);
				break;
			case 1:
				read_textwall_buffer(&offset_l, frame);
				break;
			case 2:
				read_frame_buffer(frame);
				break;
			default:
				break;
		}

		if(DEBUG_LEVEL)
		printf("Next round, frame: %d offset: %d  \n", frame, offset_l);

		offset_l++;
		frame++;

		 usleep(5000);
		 framebufferreadlock = 0;
		 usleep(2000);

		 if(frame % 120 == 0) {
			MODE = 1;
//			if(MODE == 3) {
//				MODE = 0;
//			}
			framebuffered = 1;
			usleep(500000);
		}

		while(framebufferwritelock == 1) { // it stopped writing
			usleep(1);
		}

  }
}

void *write(void *ptr)
{
	int start = 0;
	usleep(1000000);
	char * textbuffer = "BIER HALEN!! ";
    /* increment x to 100 */
    while(1) {

    	while(framebufferreadlock == 1) {
			usleep(5000);
		}
    	framebufferwritelock = 1;
    	start++;
    	switch (MODE) {
			case 0:
				write_wall_buffer(textbuffer, (int)  strlen(textbuffer), start);
				break;
			case 1:
				write_textwall_buffer(textbuffer, (int) strlen(textbuffer));
				break;
			case 2:
				write_text_to_buffer(textbuffer);
				break;
			default:
				break;
		}


	    framebufferwritelock = 0;
	    usleep(2000);
	    while(framebuffered == 0) {
	    	usleep(5000);

	    }
	    while(framebufferreadlock == 1) {
	    	usleep(5000);
	    }
	    if(DEBUG_LEVEL)
	    printf("Write %d - %d\n", framebuffered, start);
	    framebuffered=0;

	}
    return NULL;

}




void populate(GtkLayout * l) {
   
	for(int i = COLUMNS; i >= 0; i--) {
        for(int x = ROWS; x >= 0; x--) {
		    GtkWidget * lbl = gtk_label_new(" ");
	        	gtk_widget_set_size_request(lbl, 25, 25);
		    gtk_layout_put (l,
			    lbl,
			    i * 50,
			    x * 50);
		    int bx = (x - ROWS) * -1;
		    int bi = (i - COLUMNS) * -1;
		   // printf("%d - %d \r\n", bi, bx);
		    buff2[bi][bx] = lbl;
        }
	}

}

int main( int    argc, char **argv )
{
    GtkBuilder *builder;
    GtkWidget  *window;
    GError     *error = NULL;
GtkLayout * layout;

    /* Init GTK+ */
    gtk_init( &argc, &argv );

    /* Create new GtkBuilder object */
    builder = gtk_builder_new();
    /* Load UI from file. If error occurs, report it and quit application.
     * Replace "tut.glade" with your saved project. */
    if( ! gtk_builder_add_from_file( builder, "sim.glade", &error ) )
    {
        g_warning( "%s", error->message );
        g_free( error );
        return( 1 );
    }

    	/* Get main window pointer from UI */
    	window = GTK_WIDGET( gtk_builder_get_object( builder, "window1" ) );

	layout = GTK_LAYOUT(gtk_builder_get_object (builder, "layout2"));

    populate(layout);
    

    /* Connect signals */
       gtk_builder_connect_signals( builder, NULL );


       /* this variable is our reference to the second thread */
       pthread_t inc_x_thread;
    pthread_t wr_x_thread;


    if(pthread_create(&wr_x_thread, NULL, write, layout)) {

          fprintf(stderr, "Error creating thread\n");
          return 1;

      }

       /* create a second thread which executes inc_x(&x) */
       if(pthread_create(&inc_x_thread, NULL, react, layout)) {

           fprintf(stderr, "Error creating thread\n");
           return 1;

       }


   	 /* Destroy builder, since we don't need it anymore */
       g_object_unref( G_OBJECT( builder ) );

       /* Show window. All other widgets are automatically shown by GtkBuilder */
       gtk_widget_show_all( window );




    /* Start main loop */
    gtk_main();


    return( 0 );
}

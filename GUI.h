#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>
#include "GameData.h"

/* * Initializes the GTK environment, constructs the main window using GtkGrid, 
 * and enters the event-driven GTK main loop. 
 */
void StartGUI(int argc, char *argv[], Board* pBoard);

#endif // GUI_H

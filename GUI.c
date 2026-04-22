#include "GUI.h"
#include <cairo.h>

/* Global pointer to the active game state for the rendering thread */
static Board* g_pActiveBoard = NULL;

/*
 * Cairo callback triggered via the "draw" signal.
 * Responsible for rendering the 8x10 Anteater Chess grid dynamically
 * based on the current window dimensions.
 */
static gboolean OnDrawBoard(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    double cellWidth = (double)width / COLS;
    double cellHeight = (double)height / ROWS;

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if ((row + col) % 2 == 0) {
                cairo_set_source_rgb(cr, 0.93, 0.86, 0.73); 
            }
            else {
                cairo_set_source_rgb(cr, 0.46, 0.59, 0.34); 
            }
            
            /* Invert row index so rank 1 (row 0) renders at the bottom of the window */
            int renderRow = ROWS - 1 - row;
            
            cairo_rectangle(cr, col * cellWidth, renderRow * cellHeight, cellWidth, cellHeight);
            cairo_fill(cr);
        }
    }

    return FALSE;
}

/*
 * Callback for the header bar toggle button.
 * Modifies the GtkRevealer's child property to animate the sidebar in/out.
 */
static void OnSidebarToggle(GtkToggleButton *toggle_button, gpointer user_data)
{
    GtkRevealer *revealer = GTK_REVEALER(user_data);
    gboolean is_active = gtk_toggle_button_get_active(toggle_button);
    gtk_revealer_set_reveal_child(revealer, is_active);
}

void StartGUI(int argc, char *argv[], Board* pBoard)
{
    g_pActiveBoard = pBoard;
    
    gtk_init(&argc, &argv);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    /* Increased default width to accommodate the 250px sidebar alongside the board */
    gtk_window_set_default_size(GTK_WINDOW(window), 1050, 640);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    /* * Implement a GtkHeaderBar to replace the native window title bar.
     * This provides a clean location for the sidebar toggle control.
     */
    GtkWidget *headerBar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(headerBar), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(headerBar), "Anteater Chess");
    gtk_window_set_titlebar(GTK_WINDOW(window), headerBar);

    GtkWidget *toggleBtn = gtk_toggle_button_new_with_label("Move Log");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerBar), toggleBtn);

    /* Main container: Horizontal box to place sidebar and board side-by-side */
    GtkWidget *mainBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), mainBox);

    /* * GtkRevealer handles the smooth slide animation for the collapsible UI component.
     */
    GtkWidget *revealer = gtk_revealer_new();
    gtk_revealer_set_transition_type(GTK_REVEALER(revealer), GTK_REVEALER_TRANSITION_TYPE_SLIDE_RIGHT);
    gtk_revealer_set_transition_duration(GTK_REVEALER(revealer), 300);
    
    /* Pack revealer with FALSE expand/fill so it strictly respects its requested size */
    gtk_box_pack_start(GTK_BOX(mainBox), revealer, FALSE, FALSE, 0);

    g_signal_connect(toggleBtn, "toggled", G_CALLBACK(OnSidebarToggle), revealer);

    /* Scrolled window to handle future log overflow */
    GtkWidget *scrolledWin = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolledWin, 250, -1);
    gtk_container_add(GTK_CONTAINER(revealer), scrolledWin);

    /* Uneditable text view acting as the log container */
    GtkWidget *logTextView = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(logTextView), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(logTextView), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(logTextView), GTK_WRAP_WORD);
    
    /* Apply a slight margin for UX text readability */
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(logTextView), 5);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(logTextView), 5);
    gtk_container_add(GTK_CONTAINER(scrolledWin), logTextView);

    /* Core rendering area */
    GtkWidget *drawingArea = gtk_drawing_area_new();
    gtk_widget_set_hexpand(drawingArea, TRUE);
    gtk_widget_set_vexpand(drawingArea, TRUE);
    g_signal_connect(drawingArea, "draw", G_CALLBACK(OnDrawBoard), NULL);
    
    /* Pack the board with TRUE expand to consume all remaining window space */
    gtk_box_pack_start(GTK_BOX(mainBox), drawingArea, TRUE, TRUE, 0);

    /* Initialize the application state with the sidebar open */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggleBtn), TRUE);

    gtk_widget_show_all(window);
    
    gtk_main();
}

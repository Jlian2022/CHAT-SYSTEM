#include <gtk/gtk.h>

int HasLoggedIn = 0;
GtkTextBuffer *buffer;
GtkWidget *window;
GtkWidget *grid;
GtkWidget *text_view;
GtkWidget *entry;
GtkWidget *send_button;
void append_to_text_view(const char *message);
void clear_text_view();
static void show_login_dialog();
static void on_login_dialog_response();
static void on_send_button_clicked(GtkButton *button, gpointer user_data);

static void show_login_dialog()
{
    GtkWidget *dialog, *content_area, *label, *entry;

    dialog = gtk_dialog_new_with_buttons("Login", GTK_WINDOW(window),
                                         GTK_DIALOG_MODAL,
                                         "OK", GTK_RESPONSE_OK,
                                         "Cancel", GTK_RESPONSE_CANCEL,
                                         NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 150);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Add some padding to the content area
    gtk_widget_set_margin_top(content_area, 10);
    gtk_widget_set_margin_bottom(content_area, 10);
    gtk_widget_set_margin_start(content_area, 10);
    gtk_widget_set_margin_end(content_area, 10);

    label = gtk_label_new("Enter your name:");
    gtk_box_append(GTK_BOX(content_area), label);

    // Add some spacing between the label and the entry
    gtk_widget_set_margin_bottom(label, 10);

    entry = gtk_entry_new();
    gtk_box_append(GTK_BOX(content_area), entry);

    // Make the entry expand to fill available space
    gtk_widget_set_hexpand(entry, TRUE);

    g_signal_connect(dialog, "response", G_CALLBACK(on_login_dialog_response), entry);
    

    gtk_widget_show(dialog);
}

static void on_login_dialog_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
    if (response_id == GTK_RESPONSE_OK)
    {
        GtkEntry *entry = GTK_ENTRY(user_data);
        const char *name = gtk_editable_get_text(GTK_EDITABLE(entry));
        if (strlen(name) > 0)
        {
            HasLoggedIn = 1;
            g_print("Logged in as: %s\n", name);
        }
    }

    gtk_window_destroy(dialog);
}

void clear_text_view()
{
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, "", -1);
}

void append_to_text_view(const char *message)
{
    GtkTextIter end;

    // Get the buffer associated with the text view
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

    // Get the end iterator
    gtk_text_buffer_get_end_iter(buffer, &end);

    // Insert the message at the end of the buffer
    gtk_text_buffer_insert(buffer, &end, message, -1);

    // Insert a newline
    gtk_text_buffer_insert(buffer, &end, "\n", -1);

    // Scroll to the bottom of the text view
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(text_view),
                                 gtk_text_buffer_get_insert(buffer),
                                 0.0, FALSE, 0.0, 0.0);
}

static void
on_send_button_clicked(GtkButton *button, gpointer user_data)
{
    if (HasLoggedIn == 0)
    {
        show_login_dialog();
        return;
    }
    GtkEntry *entry = GTK_ENTRY(user_data);
    GtkEntryBuffer *entry_buffer = gtk_entry_get_buffer(entry);
    const gchar *text = gtk_entry_buffer_get_text(entry_buffer);

    // Append the text to the text view
    append_to_text_view(text);

    // Clear the entry
    gtk_entry_buffer_set_text(entry_buffer, "", 0);
}

static void
activate(GtkApplication *app, gpointer user_data)
{

    // Create the main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Chat Application");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Create a grid container
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), FALSE);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), FALSE);
    gtk_window_set_child(GTK_WINDOW(window), grid);

    // Create a scrolled window
    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_widget_set_hexpand(scrolled_window, TRUE);
    gtk_grid_attach(GTK_GRID(grid), scrolled_window, 0, 0, 2, 1);

    // Create the text view for displaying messages
    text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);
    gtk_widget_set_vexpand(text_view, TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD_CHAR);

    // Add the text view to the scrolled window
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), text_view);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, "\n Welcome !!!\n", -1);

    // Create the entry for user input
    entry = gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);
    gtk_entry_set_placeholder_text(entry, "Enter message here");

    // Create the send button
    send_button = gtk_button_new_with_label("Send");
    gtk_grid_attach(GTK_GRID(grid), send_button, 1, 1, 1, 1);

    // Store the text view in the send button's data for use in the callback
    g_object_set_data(G_OBJECT(send_button), "text_view", text_view);

    // Connect the clicked signal of the send button to the callback function

    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_button_clicked), entry);

    // Connect the activate signal of the entry to the same callback function
    g_signal_connect(entry, "activate", G_CALLBACK(on_send_button_clicked), entry);

    // Present the window
    gtk_window_present(GTK_WINDOW(window));

    if (!HasLoggedIn)
    {
        show_login_dialog();
    }
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.chatapp", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

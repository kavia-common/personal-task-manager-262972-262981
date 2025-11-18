#include <gtk/gtk.h>

/*
 PUBLIC_INTERFACE
 main entry point for the To-Do native app.

 This is a minimal scaffold GTK4 application that opens a simple window.
*/

static void on_activate(GApplication *app, gpointer /*user_data*/) {
    GtkWidget *win = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(win), "ToDo - Native");
    gtk_window_set_default_size(GTK_WINDOW(win), 480, 320);

    // Simple content: a label
    GtkWidget *label = gtk_label_new("To-Do App (Scaffold)");
    gtk_window_set_child(GTK_WINDOW(win), label);

    gtk_widget_show(win);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.example.todo", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}

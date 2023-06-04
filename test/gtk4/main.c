#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Meu Programa GTK 4");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);

    GtkWidget *button = gtk_button_new_with_label("Clique Aqui");
    gtk_window_set_child(GTK_WINDOW(window), button);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_widget_destroyed), &app);

    gtk_widget_show(window);
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("com.example.myapp", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}


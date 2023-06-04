#include <gtk/gtk.h>
int main(int argc, char *argv[]) {
    // Inicialização do GTK
    gtk_init(&argc, &argv);

    // Criação da janela
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Minha Primeira ");
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Exibição da janela
    gtk_widget_show_all(window);

    // Execução do loop principal do GTK
    gtk_main();

    return 0;
}



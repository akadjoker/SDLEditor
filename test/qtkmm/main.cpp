#include <gtkmm.h>

class MyWindow : public Gtk::Window {
public:
    MyWindow() {
        set_title("Exemplo GTKMM");
        set_default_size(350, 200);

        add(m_box);

   
        m_button.signal_clicked().connect([this] { on_button_clicked(); });

        m_label.set_text("Olá, mundo!");
        m_box.pack_start(m_button);
        m_box.pack_start(m_label);

        show_all_children();
    }

protected:
    Gtk::Label m_label;
    Gtk::Box m_box{Gtk::ORIENTATION_VERTICAL};
     Gtk::Button m_button;

private:
    void on_button_clicked() 
    {
        m_label.set_text("Botão clicado!");
    }
};

int main(int argc, char *argv[]) {
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "com.exemplo.gtkmm-tutorial");

    MyWindow window;
    return app->run(window);
}

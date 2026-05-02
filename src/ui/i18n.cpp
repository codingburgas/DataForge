#include "ui/i18n.h"

namespace ui {

    namespace {

        Language gLang = LANG_EN;

        const char* gEN[K_KEY_COUNT] = {
            "File",
            "New",
            "Open...",
            "Save",
            "Save As...",
            "Exit",

            "Edit",
            "New Task",
            "Edit Selected...",
            "Delete Selected",
            "Undo",

            "View",
            "Overview",
            "My Tasks",
            "Analytics",
            "Benchmark",
            "Light Theme",
            "Dark Theme",

            "Algorithms",
            "Priority order",
            "Deadline order",
            "Quick sort",
            "Bubble sort",

            "Language",
            "English",
            "Spanish",

            "Help",
            "User Guide",
            "About DataForge",

            "Undid last action.",

            "Overview",
            "My Tasks",
            "Analytics",
            "Benchmark",
            "Help",

            "Overview",
            "My Tasks",
            "Analytics",
            "Benchmark",
            "Help",
            "Workspace",

            "Here is what is moving across the workspace.",
            "Manage hierarchy, focus work, and edit task details.",
            "Track delivery health, workload mix, and project progress.",
            "Compare sorting strategies on synthetic task sets.",
            "Workspace guides and recovery tools for lost task files.",

            "Good morning",
            "Good afternoon",
            "Good evening",

            "Today",
            "Here is what is on your plate today.",
            "Unsaved",
            "Ready",
            "Saved",
            "+  New Task",

            "Workspace",
            "%d root projects",
            "%d tasks across the current store",
            "Collapse",
            "Expand",

            "No file selected",
            "%d overdue",
            "%d tasks"
        };

        const char* gES[K_KEY_COUNT] = {
            "Archivo",
            "Nuevo",
            "Abrir...",
            "Guardar",
            "Guardar como...",
            "Salir",

            "Editar",
            "Nueva tarea",
            "Editar seleccionada...",
            "Eliminar seleccionada",
            "Deshacer",

            "Vista",
            "Resumen",
            "Mis tareas",
            "Analitica",
            "Prueba",
            "Tema claro",
            "Tema oscuro",

            "Algoritmos",
            "Orden por prioridad",
            "Orden por fecha limite",
            "Ordenamiento rapido",
            "Ordenamiento burbuja",

            "Idioma",
            "Ingles",
            "Espanol",

            "Ayuda",
            "Guia del usuario",
            "Acerca de DataForge",

            "Se deshizo la ultima accion.",

            "Resumen",
            "Mis tareas",
            "Analitica",
            "Prueba",
            "Ayuda",

            "Resumen",
            "Mis tareas",
            "Analitica",
            "Prueba de rendimiento",
            "Ayuda",
            "Espacio de trabajo",

            "Esto es lo que esta avanzando en el espacio de trabajo.",
            "Gestiona la jerarquia, enfoca el trabajo y edita los detalles.",
            "Sigue la salud del proyecto, la carga y el progreso.",
            "Compara estrategias de ordenamiento con conjuntos sinteticos.",
            "Guias del espacio y herramientas para recuperar archivos perdidos.",

            "Buenos dias",
            "Buenas tardes",
            "Buenas noches",

            "Hoy",
            "Esto es lo que tienes para hoy.",
            "Sin guardar",
            "Listo",
            "Guardado",
            "+  Nueva tarea",

            "Espacio",
            "%d proyectos raiz",
            "%d tareas en el archivo actual",
            "Contraer",
            "Expandir",

            "Ningun archivo seleccionado",
            "%d atrasadas",
            "%d tareas"
        };

    }

    void setLanguage(Language lang) {
        gLang = lang;
    }

    Language currentLanguage() {
        return gLang;
    }

    const char* tr(StringKey key) {
        if (key < 0 || key >= K_KEY_COUNT) {
            return "";
        }
        const char** table = (gLang == LANG_ES) ? gES : gEN;
        const char* s = table[key];
        if (s == nullptr) {
            s = gEN[key];
        }
        return s == nullptr ? "" : s;
    }

}

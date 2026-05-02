#include "ui/i18n.h"

namespace ui {

    namespace {

        Language gLang = LANG_EN;

        const char* gEN[K_KEY_COUNT] = {
            // menu file
            "File", "New", "Open...", "Save", "Save As...", "Exit",
            // menu edit
            "Edit", "New Task", "Edit Selected...", "Delete Selected", "Undo",
            // menu view
            "View", "Overview", "My Tasks", "Analytics", "Benchmark",
            "Light Theme", "Dark Theme",
            // menu algorithms
            "Algorithms", "Priority order", "Deadline order", "Quick sort", "Bubble sort",
            // menu language
            "Language", "English", "Spanish",
            // menu help
            "Help", "User Guide", "About DataForge",

            // toasts
            "Undid last action.",
            "Task updated.",
            "Task created.",
            "Deleted %d task.",
            "Deleted %d tasks.",
            "Benchmark complete.",
            "Documents folder not found.",
            "Scan complete: %zu file(s) found.",

            // nav
            "Overview", "My Tasks", "Analytics", "Benchmark", "Help",
            // page titles
            "Overview", "My Tasks", "Analytics", "Benchmark", "Help", "Workspace",
            // subtitles
            "Here is what is moving across the workspace.",
            "Manage hierarchy, focus work, and edit task details.",
            "Track delivery health, workload mix, and project progress.",
            "Compare sorting strategies on synthetic task sets.",
            "Workspace guides and recovery tools for lost task files.",
            // greetings
            "Good morning", "Good afternoon", "Good evening",
            // top bar
            "Today",
            "Here is what is on your plate today.",
            "Unsaved", "Ready", "Saved", "+  New Task",
            // sidebar
            "Workspace",
            "%d root projects",
            "%d tasks across the current store",
            "Collapse", "Expand",
            // status bar
            "No file selected", "%d overdue", "%d tasks",

            // overview
            "Total Tasks", "Completed", "Active Projects", "Blocked",
            "In progress now", "Quiet queue", "Shipping clean work", "Root task trees",
            "Needs attention", "No blockers",
            "Today's Focus", "+ Add task",
            "No active tasks yet. Create one to start the board.",
            "Active Projects", "No root projects yet.",
            "Recent Activity", "Nothing recent yet.", "Updated recently",
            "No task notes yet.", "No due date",
            "%d subtasks  |  %.0f%% done",

            // tree
            "(depth cap)", "Project Tree", "%d root items",
            "No tasks yet.", "No root tasks.",
            "%d descendants  |  depth %d",

            // table
            "All priorities", "All statuses",
            "Task Queue", "%d results",
            "Search, filter, and rank work without leaving the main view.",
            "Search tasks by title...",
            "No description provided.",
            "%d min", "Actual %d min",
            "%s  |  %d subtasks",
            "No due date",
            "Nothing matches the current view",
            "Try clearing filters or add a new task from the top bar.",

            // priority/status labels
            "Low", "Medium", "High", "Critical",
            "To Do", "In Progress", "Done", "Blocked",
            "Priority", "Deadline",
            "Quick", "Bubble",

            // task details
            "Task Details",
            "Selection summary, subtree metrics, and task actions.",
            "Select a task to inspect it.",
            "Task no longer exists.",
            "No description yet.",
            "Deadline", "Estimated", "Actual", "Subtasks", "Depth", "Subtree effort",
            "No due date",
            "Created %s", "Updated %s",
            "Edit", "Add Subtask", "Delete",

            // dialogs
            "Edit Task", "Create Task",
            "Capture the work clearly so the queue stays readable.",
            "Title", "Description", "Priority", "Status", "Deadline (YYYY-MM-DD)",
            "Estimated minutes", "Actual minutes", "Parent task",
            "Cancel", "Save Changes", "Create Task",
            "(root)", "(invalid)",

            "Delete task",
            "This action removes the selected task and all of its descendants.",
            "Task no longer exists.",
            "Also deletes %d subtasks.",
            "No subtasks will be affected.",
            "Delete", "Cancel",

            "Unsaved changes",
            "Save the current store before closing the application?",
            "Save and quit", "Quit without saving", "Cancel",

            "Hierarchical task manager built with Dear ImGui, Win32, and DirectX 11.",
            "Key traits",
            "Task trees, analytics, algorithm benchmarking, and keyboard-first editing.",
            "This redesign consolidates the workspace into real pages instead of modal-heavy navigation.",
            "Close",

            // stats
            "Workspace Analytics",
            "Measure task mix, delivery risk, and project completion in one place.",
            "Total Tasks", "Completed", "Root Projects", "Overdue",
            "Finished work", "Top-level trees", "Needs attention", "Healthy",
            "Priority distribution", "Status distribution", "Project health",
            "No root projects yet.",
            "%dh %02dm  |  %d subtasks  |  depth %d",
            "Signals",
            "Overdue pressure",
            "Tasks are overdue and should be reviewed.",
            "No overdue items right now.",
            "Execution mix",
            "%.0f in progress", "%.0f blocked", "%.0f done",
            "Use this to gauge whether work is flowing or stalling.",

            // benchmark
            "Sorting Benchmark",
            "Stress test the built-in sort implementations with the current workload size.",
            "Run comparison",
            "Quick sort should scale far better than bubble sort once the list grows.",
            "Item count",
            "Run Benchmark",
            "Use the result to explain why the UI defaults to the faster algorithm.",
            "Results",
            "No run yet. Set the item count and execute the benchmark.",
            "Items tested: %d",
            "Bubble", "Quick",
            "Takeaway",
            "Quick sort is %.1fx faster on this run.",
            "Bubble sort is still useful here as a teaching tool, but it should not be the default path.",

            // help
            "UI Guide", "Walkthrough of every part of the workspace.",
            "Find lost task files",
            "Searches your Documents folder (recursively) for *.dftasks files.",
            "Scan Documents",
            "Searched: %s",
            "(no Documents folder)",
            "Click to search %%USERPROFILE%%\\Documents.",
            "Results appear here. Each row has a Load button.",
            "Documents folder could not be located on this profile.",
            "No .dftasks files were found under Documents.",
            "%zu file(s) found, sorted by most recently modified.",
            "Load", "unknown",

            "Sidebar",
                "Switch between Overview, My Tasks, Analytics, Benchmark, and Help. Click 'Collapse' at the bottom to shrink the sidebar to icon-only.",
            "Topbar",
                "Shows the current page title, today's date, and an unsaved/ready indicator. The '+ New Task' button opens the create dialog.",
            "Overview page",
                "Greeting, four metric cards (total, completed, projects, blocked), the Today's Focus list of unfinished priority tasks, and side cards for active root projects and recent activity. Click any card to jump to My Tasks with that task selected.",
            "My Tasks page",
                "Three panes: tree on the left for hierarchy navigation, table in the middle for filtering and sorting, details on the right for editing the selected task. Use the search box and priority/status filters above the table to narrow the list.",
            "Task tree",
                "Expand/collapse parent tasks. Selecting a node updates the table and details panes. Right-click a node for create/delete actions.",
            "Task table",
                "Click a row to select. Header sort matches the Algorithms menu. Search box matches title and description.",
            "Task details",
                "Read-only view with progress, deadline urgency, descendants count, and shortcuts to edit or delete. Edit opens the same dialog as 'New Task'.",
            "Analytics page",
                "Charts and counts for status mix, priority mix, and completion trends across the current store.",
            "Benchmark page",
                "Generates a synthetic task list of the chosen size and times Quick sort vs Bubble sort on it. Use to compare algorithm behavior on the same data.",
            "Status bar",
                "Bottom strip with file path, dirty indicator, and last toast message.",
            "Menu bar",
                "File: New / Open / Save / Save As / Exit. Edit: New Task, edit/delete selected, Undo. View: switch page or theme. Algorithms: sort key and algorithm. Help: this page and About.",
            "Voice input",
                "The mic icon next to Title and Description fields records speech and inserts the transcript. Requires Windows speech to be available; failures show as a toast on launch.",
            "Keyboard shortcuts",
                "Ctrl+N new file, Ctrl+O open, Ctrl+S save, Ctrl+Shift+S save as, Ctrl+Enter new task, Del delete selected, Ctrl+Z undo last destructive action."
        };

        const char* gES[K_KEY_COUNT] = {
            // menu file
            "Archivo", "Nuevo", "Abrir...", "Guardar", "Guardar como...", "Salir",
            // menu edit
            "Editar", "Nueva tarea", "Editar seleccionada...", "Eliminar seleccionada", "Deshacer",
            // menu view
            "Vista", "Resumen", "Mis tareas", "Analitica", "Prueba",
            "Tema claro", "Tema oscuro",
            // menu algorithms
            "Algoritmos", "Orden por prioridad", "Orden por fecha limite",
            "Ordenamiento rapido", "Ordenamiento burbuja",
            // menu language
            "Idioma", "Ingles", "Espanol",
            // menu help
            "Ayuda", "Guia del usuario", "Acerca de DataForge",

            // toasts
            "Se deshizo la ultima accion.",
            "Tarea actualizada.",
            "Tarea creada.",
            "%d tarea eliminada.",
            "%d tareas eliminadas.",
            "Prueba completada.",
            "Carpeta Documentos no encontrada.",
            "Escaneo completo: %zu archivo(s) encontrados.",

            // nav
            "Resumen", "Mis tareas", "Analitica", "Prueba", "Ayuda",
            // page titles
            "Resumen", "Mis tareas", "Analitica", "Prueba de rendimiento", "Ayuda", "Espacio de trabajo",
            // subtitles
            "Esto es lo que esta avanzando en el espacio de trabajo.",
            "Gestiona la jerarquia, enfoca el trabajo y edita los detalles.",
            "Sigue la salud del proyecto, la carga y el progreso.",
            "Compara estrategias de ordenamiento con conjuntos sinteticos.",
            "Guias del espacio y herramientas para recuperar archivos perdidos.",
            // greetings
            "Buenos dias", "Buenas tardes", "Buenas noches",
            // top bar
            "Hoy",
            "Esto es lo que tienes para hoy.",
            "Sin guardar", "Listo", "Guardado", "+  Nueva tarea",
            // sidebar
            "Espacio",
            "%d proyectos raiz",
            "%d tareas en el archivo actual",
            "Contraer", "Expandir",
            // status bar
            "Ningun archivo seleccionado", "%d atrasadas", "%d tareas",

            // overview
            "Tareas totales", "Completadas", "Proyectos activos", "Bloqueadas",
            "En progreso ahora", "Cola tranquila", "Entregando trabajo limpio", "Arboles raiz",
            "Necesita atencion", "Sin bloqueos",
            "Enfoque de hoy", "+ Agregar tarea",
            "No hay tareas activas. Crea una para comenzar.",
            "Proyectos activos", "Aun no hay proyectos raiz.",
            "Actividad reciente", "Nada reciente todavia.", "Actualizada recientemente",
            "Aun no hay notas.", "Sin fecha limite",
            "%d subtareas  |  %.0f%% completado",

            // tree
            "(limite de profundidad)", "Arbol de proyectos", "%d elementos raiz",
            "Aun no hay tareas.", "No hay tareas raiz.",
            "%d descendientes  |  profundidad %d",

            // table
            "Todas prioridades", "Todos estados",
            "Cola de tareas", "%d resultados",
            "Busca, filtra y ordena el trabajo sin salir de la vista.",
            "Buscar tareas por titulo...",
            "Sin descripcion.",
            "%d min", "Real %d min",
            "%s  |  %d subtareas",
            "Sin fecha limite",
            "Nada coincide con la vista actual",
            "Limpia los filtros o agrega una tarea desde la barra superior.",

            // priority/status labels
            "Baja", "Media", "Alta", "Critica",
            "Por hacer", "En progreso", "Hecha", "Bloqueada",
            "Prioridad", "Fecha limite",
            "Rapido", "Burbuja",

            // task details
            "Detalles de tarea",
            "Resumen, metricas del subarbol y acciones.",
            "Selecciona una tarea para inspeccionarla.",
            "La tarea ya no existe.",
            "Aun sin descripcion.",
            "Fecha limite", "Estimado", "Real", "Subtareas", "Profundidad", "Esfuerzo subarbol",
            "Sin fecha limite",
            "Creada %s", "Actualizada %s",
            "Editar", "Agregar subtarea", "Eliminar",

            // dialogs
            "Editar tarea", "Crear tarea",
            "Captura el trabajo claramente para mantener la cola legible.",
            "Titulo", "Descripcion", "Prioridad", "Estado", "Fecha limite (AAAA-MM-DD)",
            "Minutos estimados", "Minutos reales", "Tarea padre",
            "Cancelar", "Guardar cambios", "Crear tarea",
            "(raiz)", "(invalida)",

            "Eliminar tarea",
            "Esta accion elimina la tarea seleccionada y todos sus descendientes.",
            "La tarea ya no existe.",
            "Tambien elimina %d subtareas.",
            "Ninguna subtarea sera afectada.",
            "Eliminar", "Cancelar",

            "Cambios sin guardar",
            "Guardar el archivo actual antes de cerrar la aplicacion?",
            "Guardar y salir", "Salir sin guardar", "Cancelar",

            "Gestor de tareas jerarquicas hecho con Dear ImGui, Win32 y DirectX 11.",
            "Caracteristicas",
            "Arboles de tareas, analitica, comparacion de algoritmos y edicion por teclado.",
            "Este rediseno consolida el espacio en paginas reales en lugar de navegacion modal.",
            "Cerrar",

            // stats
            "Analitica del espacio",
            "Mide la mezcla de tareas, riesgo de entrega y completado de proyectos.",
            "Tareas totales", "Completadas", "Proyectos raiz", "Atrasadas",
            "Trabajo terminado", "Arboles raiz", "Necesita atencion", "Saludable",
            "Distribucion por prioridad", "Distribucion por estado", "Salud del proyecto",
            "Aun no hay proyectos raiz.",
            "%dh %02dm  |  %d subtareas  |  profundidad %d",
            "Senales",
            "Presion de atrasos",
            "Hay tareas atrasadas y deben revisarse.",
            "Sin tareas atrasadas ahora mismo.",
            "Mezcla de ejecucion",
            "%.0f en progreso", "%.0f bloqueadas", "%.0f hechas",
            "Usa esto para evaluar si el trabajo fluye o se detiene.",

            // benchmark
            "Prueba de ordenamiento",
            "Pon a prueba las implementaciones de ordenamiento con la carga actual.",
            "Ejecutar comparacion",
            "Quick sort deberia escalar mucho mejor que bubble sort al crecer la lista.",
            "Cantidad de elementos",
            "Ejecutar prueba",
            "Usa el resultado para explicar por que la UI usa el algoritmo mas rapido.",
            "Resultados",
            "Sin ejecuciones aun. Define la cantidad y ejecuta la prueba.",
            "Elementos probados: %d",
            "Burbuja", "Rapido",
            "Conclusion",
            "Quick sort es %.1fx mas rapido en esta ejecucion.",
            "Bubble sort sirve como herramienta didactica, pero no debe ser la opcion por defecto.",

            // help
            "Guia de la UI", "Recorrido por cada parte del espacio de trabajo.",
            "Buscar archivos perdidos",
            "Busca archivos *.dftasks dentro de tu carpeta Documentos (recursivo).",
            "Escanear Documentos",
            "Buscado: %s",
            "(sin carpeta Documentos)",
            "Haz clic para buscar en %%USERPROFILE%%\\Documentos.",
            "Los resultados aparecen aqui. Cada fila tiene un boton Cargar.",
            "No se pudo localizar la carpeta Documentos.",
            "No se encontraron archivos .dftasks en Documentos.",
            "%zu archivo(s) encontrados, ordenados por modificacion mas reciente.",
            "Cargar", "desconocido",

            "Barra lateral",
                "Cambia entre Resumen, Mis tareas, Analitica, Prueba y Ayuda. Haz clic en 'Contraer' para reducirla a iconos.",
            "Barra superior",
                "Muestra el titulo de la pagina, fecha de hoy y un indicador de guardado. El boton '+ Nueva tarea' abre el dialogo de creacion.",
            "Pagina Resumen",
                "Saludo, cuatro tarjetas (totales, completadas, proyectos, bloqueadas), lista 'Enfoque de hoy' y tarjetas laterales de proyectos y actividad reciente. Haz clic en cualquier tarjeta para saltar a Mis tareas con ella seleccionada.",
            "Pagina Mis tareas",
                "Tres paneles: arbol a la izquierda para jerarquia, tabla central para filtrar y ordenar, detalles a la derecha para editar. Usa la busqueda y los filtros encima de la tabla para acotar la lista.",
            "Arbol de tareas",
                "Expande/contrae padres. Seleccionar un nodo actualiza la tabla y los detalles. Clic derecho para crear o eliminar.",
            "Tabla de tareas",
                "Haz clic en una fila para seleccionar. El orden coincide con el menu Algoritmos. La busqueda compara titulo y descripcion.",
            "Detalles de tarea",
                "Vista de solo lectura con progreso, urgencia, descendientes y atajos para editar o eliminar. Editar abre el mismo dialogo que 'Nueva tarea'.",
            "Pagina Analitica",
                "Graficas y conteos por estado, prioridad y tendencias de completado del archivo actual.",
            "Pagina Prueba",
                "Genera una lista sintetica del tamano elegido y mide Quick sort vs Bubble sort. Sirve para comparar algoritmos sobre los mismos datos.",
            "Barra de estado",
                "Tira inferior con ruta de archivo, indicador de cambios y ultimo aviso.",
            "Barra de menus",
                "Archivo: Nuevo / Abrir / Guardar / Guardar como / Salir. Editar: Nueva tarea, editar/eliminar seleccionada, Deshacer. Vista: cambia pagina o tema. Algoritmos: clave y algoritmo. Ayuda: esta pagina y Acerca de.",
            "Entrada por voz",
                "El icono de microfono junto a Titulo y Descripcion graba voz e inserta la transcripcion. Requiere Windows Speech; las fallas aparecen como aviso al iniciar.",
            "Atajos de teclado",
                "Ctrl+N nuevo, Ctrl+O abrir, Ctrl+S guardar, Ctrl+Shift+S guardar como, Ctrl+Enter nueva tarea, Del eliminar seleccionada, Ctrl+Z deshacer."
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

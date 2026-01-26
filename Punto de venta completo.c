#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <string.h>

#define MAX_PRODUCTOS 100
#define MAX_CARRITO 50

// Prototipos de funciones


int ivaGlobal = 12;
int numeroDia = 1;
int facturaActual = 1;
int cantidadProductos;
int limiteStock = 5;

typedef struct {
    int facturaID;
    char fecha[11]; 
    char hora[9];   
    int dia;
    int codigo;
    char nombre[30];
    int cantidad;
    float precioUnitario;
    float subtotal;
    float iva;
    float totalFactura;
} FacturaDetalle;


typedef struct {
    int codigo;
    char nombre[30];
    int cantidadTotal;
    float ingresoTotal;
} ResumenProducto;

typedef struct { //estructura de productos
    int codigo;
    char nombre[30];
    int stock;
    float precio;
} Producto;

Producto productos[MAX_PRODUCTOS];


int cajaAbierta = 0;   



typedef struct {
    int codigo;
    int cantidad;
    float precioUnitario;
} ItemCarrito;
void eliminarSalto();
void guardaConfiguracion(void); 
void buscarProductosPorNombre();
void buscarProductosPorCodigo();
void convertirMayusculas(char *str);
void cargarConfig();
void eliminarProducto();
void obtenerFechaHora(char *fecha, char *hora) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    strftime(fecha, 11, "%Y-%m-%d", tm_info);
    strftime(hora, 9, "%H:%M:%S", tm_info);
}

void cargarProductos() {
    FILE *f = fopen("productos.csv", "r");
    if (f == NULL) {
        return;
    }
    char linea[200];
    cantidadProductos = 0;

    fgets(linea, sizeof(linea), f);

    while (fgets(linea, sizeof(linea), f)) {

        Producto p;
        char *token = strtok(linea, ",");

        if (!token) {
            continue;
        }    
        p.codigo = atoi(token);

        token = strtok(NULL, ",");
        strcpy(p.nombre, token);
        convertirMayusculas(p.nombre);

        token = strtok(NULL, ",");
        p.stock = atoi(token);

        token = strtok(NULL, ",");
        p.precio = atof(token);

        productos[cantidadProductos++] = p;
    }

    fclose(f);
}

 void guardarProductos() {
    FILE *f = fopen("productos.csv", "w");
    if (f == NULL) {
        printf("Error al guardar productos\n");
        return;
    }


    fprintf(f, "codigo,nombre,stock,precio\n");

    for (int i = 0; i < cantidadProductos; i++) {
        fprintf(f, "%d,%s,%d,%.2f\n",
                productos[i].codigo,
                productos[i].nombre,
                productos[i].stock,
                productos[i].precio);
    }

    fclose(f);
}

void guardarFactura(ItemCarrito carrito[], int items, float ivaTotal, float total) {

    cargarConfig();

    FILE *f = fopen("facturas.csv", "a");
    if (f == NULL) {
        printf("Error al guardar factura\n");
        return;
    }
    ivaTotal = ivaGlobal;



    char fecha[11], hora[9];
    obtenerFechaHora(fecha, hora);

    for (int i = 0; i < items; i++) {
        int idx = -1;
        for (int j = 0; j < cantidadProductos; j++) {
            if (productos[j].codigo == carrito[i].codigo) {
                idx = j;
                break;
            }
        }
        if (idx == -1) {
            continue;
        }

        float subtotalProducto = carrito[i].cantidad * carrito[i].precioUnitario;

        fprintf(f,
            "%d,%s,%s,%d,%d,%s,%d,%.2f,%.2f,%.2f,%.2f\n",
            facturaActual,
            fecha,
            hora,
            numeroDia,
            carrito[i].codigo,
            productos[idx].nombre,
            carrito[i].cantidad,
            carrito[i].precioUnitario,
            subtotalProducto,
            ivaTotal,
            total
        );
    }
    facturaActual++;
    fclose(f);
    guardaConfiguracion();
    
}

void bajoStock() {
    int idx = -1 ;
    for(int i = 0; i < cantidadProductos; i++){
        if(productos[i].stock <= limiteStock){
            printf("Estock bajo de %s y es: %d\n", productos[i].nombre, productos[i].stock);
            idx = i ;
        }
    }
    if (idx == -1) {
        printf("Ningun producto tiene bajo stock. \n") ;
    }

    printf("El stock de todos los productos es: \n");
    for(int i = 0; i < cantidadProductos; i++){
            printf("El stock de %s es: %d\n", productos[i].nombre, productos[i].stock);
    }        
}

void venderCarrito() {

    if (!cajaAbierta) {
        printf("ERROR: Debe abrir la caja antes de vender.\n");
        return;
    }

    ItemCarrito carrito[MAX_CARRITO];
    int items = 0;
    int seguir = 1;

    while (seguir == 1 && items < MAX_CARRITO) {

        int codigo, cantidad;
        printf("\nIngrese el código del producto: ");
        scanf("%d", &codigo);

        // Buscar producto
        int idx = -1;
        for (int i = 0; i < cantidadProductos; i++) {
            if (productos[i].codigo == codigo) {
                idx = i;
                break;
            }
        }

        if (idx == -1) {
            printf("Producto no encontrado.\n");
            continue;
        }

        printf("Ingrese cantidad a comprar de %s: ", productos[idx].nombre);
        scanf("%d", &cantidad);

        if (cantidad > productos[idx].stock) {
            printf("Stock insuficiente. Disponible: %d\n", productos[idx].stock);
            continue;
        }

        carrito[items].codigo = codigo;
        carrito[items].cantidad = cantidad;
        carrito[items].precioUnitario = productos[idx].precio;
        productos[idx].stock -= cantidad ;
        items++;

        printf("¿Agregar otro producto? (1=Sí / 0=No): ");
        scanf("%d", &seguir);
    }

    // -----------------------------
    //     FACTURA FINAL
    // -----------------------------
    float subtotal = 0, ivaTotal = 0;

    printf("\n\n=========== FACTURA ===========\n");

    for (int i = 0; i < items; i++) {

        int idx = -1;
        for (int j = 0; j < cantidadProductos; j++) {
            if (productos[j].codigo == carrito[i].codigo) {
                idx = j;
                break;
            }
        }

        float sub = carrito[i].cantidad * carrito[i].precioUnitario;
        float iva = sub * (ivaGlobal / 100.0);

        subtotal += sub;
        ivaTotal += iva;

        printf("%s  x%d   $%.2f  = > Subtotal: %.2f\n",
               productos[idx].nombre,
               carrito[i].cantidad,
               carrito[i].precioUnitario,
               sub
        );
    }

    float total = subtotal + ivaTotal;
    guardarFactura(carrito, items, ivaTotal, total);
    guardarProductos();


    printf("-------------------------------\n");
    printf("Subtotal     : %.2f\n", subtotal);
    printf("IVA (%d%%)    : %.2f\n", ivaGlobal, ivaTotal);
    printf("TOTAL A PAGAR: %.2f\n", total);
    printf("===============================\n");

    printf("\nVenta completada.\n");

    // Mostrar alertas de bajo stock
    for(int i = 0; i < cantidadProductos; i++){
        if(productos[i].stock <= limiteStock){
            printf("ALERTA: Bajo stock de %s (%d unidades)\n",
                   productos[i].nombre, productos[i].stock);
        }
    }
}

void mostrarFacturaPorID(int id) {
    FILE *f = fopen("facturas.csv", "r");
    if (f == NULL) {
        printf("No hay facturas registradas.\n");
        return;
    }

    char linea[300];
    fgets(linea, sizeof(linea), f);
    float subtotalTotal = 0;
    float ivaTotal = 0;
    float totalTotal = 0;
    int validador = 0;
    printf("\n=== FACTURA #%d ===\n", id);

    while (fgets(linea, sizeof(linea), f)) {

        FacturaDetalle fd;
        char *token = strtok(linea, ",");

        fd.facturaID = atoi(token);
        token = strtok(NULL, ","); strcpy(fd.fecha, token);
        token = strtok(NULL, ","); strcpy(fd.hora, token);
        token = strtok(NULL, ","); fd.dia = atoi(token);
        token = strtok(NULL, ","); fd.codigo = atoi(token);
        token = strtok(NULL, ","); strcpy(fd.nombre, token);
        token = strtok(NULL, ","); fd.cantidad = atoi(token);
        token = strtok(NULL, ","); fd.precioUnitario = atof(token);
        token = strtok(NULL, ","); fd.subtotal = atof(token);
        token = strtok(NULL, ","); fd.iva = atof(token);
        token = strtok(NULL, ","); fd.totalFactura = atof(token);

        if (fd.facturaID == id) {
            printf("%s  x%d  $%.2f  Subtotal: $%.2f\n",
                   fd.nombre, fd.cantidad, fd.precioUnitario, fd.subtotal);

            subtotalTotal += fd.subtotal;
            ivaTotal = fd.iva;
            totalTotal = fd.totalFactura;
            validador = 1;
        }
        else{
            continue;
        }
    }

    if(validador == 0){
        printf("Factura no encontrada\n");
        fclose(f);
        return;
    }

    printf("\nSubtotal: %.2f\n", subtotalTotal);
    printf("IVA: %.2f %% \n", ivaTotal);
    printf("TOTAL: %.2f\n", totalTotal);

    fclose(f);
}


void reportePorDia(int diaBuscado) {
    FILE *f = fopen("facturas.csv", "r");
    if (f == NULL) {
        printf("No hay facturas registradas.\n");
        return;
    }

    ResumenProducto resumen[100];
    int cantidadResumen = 0;

    char linea[300];
    char fechaDia[11] = "";

    fgets(linea, sizeof(linea), f);

    while (fgets(linea, sizeof(linea), f)) {
        int dia, codigo, cantidad;
        char fecha[11], nombre[30];
        float subtotal;

        char *token = strtok(linea, ",");
        
  
        token = strtok(NULL, ","); 
        strcpy(fecha, token);

        token = strtok(NULL, ",");
        
        token = strtok(NULL, ","); 
        dia = atoi(token);

        token = strtok(NULL, ","); 
        codigo = atoi(token);

        token = strtok(NULL, ","); 
        strcpy(nombre, token);

        token = strtok(NULL, ","); 
        cantidad = atoi(token);


        token = strtok(NULL, ",");

        token = strtok(NULL, ","); 
        subtotal = atof(token);

        
        token = strtok(NULL, ","); 
        token = strtok(NULL, ","); 

        if (dia != diaBuscado)
            continue;

        if (fechaDia[0] == '\0')
            strcpy(fechaDia, fecha);

        int idx = -1;
        for (int i = 0; i < cantidadResumen; i++) {
            if (resumen[i].codigo == codigo) {
                idx = i;
                break;
            }
        }

        if (idx == -1) {
            resumen[cantidadResumen].codigo = codigo;
            strcpy(resumen[cantidadResumen].nombre, nombre);
            resumen[cantidadResumen].cantidadTotal = cantidad;
            resumen[cantidadResumen].ingresoTotal = subtotal;
            cantidadResumen++;
        } else {
            resumen[idx].cantidadTotal += cantidad;
            resumen[idx].ingresoTotal += subtotal;
        }
    }

    fclose(f);

    if (cantidadResumen == 0) {
        printf("No hubo ventas en el día %d\n", diaBuscado);
        return;
    }

    // Mostrar reporte simplificado
    printf("\n=== REPORTE DEL DÍA %d ===\n", diaBuscado);
    printf("Fecha: %s\n\n", fechaDia);
    printf("%-20s %-15s %-10s\n", "Producto", "Cantidad", "Ingreso");
    printf("------------------------------------------------\n");

    float totalDia = 0;

    for (int i = 0; i < cantidadResumen; i++) {
        printf("%-20s %-15d $%.2f\n",
               resumen[i].nombre,
               resumen[i].cantidadTotal,
               resumen[i].ingresoTotal);

        totalDia += resumen[i].ingresoTotal;
    }

    printf("------------------------------------------------\n");
    printf("INGRESO TOTAL DEL DÍA: $%.2f\n", totalDia);
}

void crearProducto() {
    Producto p;
    if (cantidadProductos == 0) {
        p.codigo = 101; 
    } else {
        int maxCodigo = productos[0].codigo;
        for (int i = 1; i < cantidadProductos; i++) {
            if (productos[i].codigo > maxCodigo) {
                maxCodigo = productos[i].codigo;
            }
        }
        p.codigo = maxCodigo + 1;
    }

    printf("Ingrese nombre: ");
    scanf(" %[^\n]", p.nombre);
    convertirMayusculas(p.nombre);

    printf("Ingrese precio: ");
    scanf("%f", &p.precio);
    
    printf("Ingrese el stock: ");
    scanf("%d", &p.stock);
    
    // Guardar en el arreglo
    productos[cantidadProductos] = p;
    cantidadProductos++;
    guardarProductos();
    printf("\nProducto registrado exitosamente.\n");
    printf("El codigo asignado es: %d\n", p.codigo);
}

void eliminarProducto() {
    if (cantidadProductos == 0) {
        printf("No hay productos registrados.\n");
        return;
    }
    
    int codigo;
    printf("\n=== 2NAR PRODUCTO ===\n");
    
    // Mostrar productos actuales
    printf("Productos actuales:\n");
    for (int i = 0; i < cantidadProductos; i++) {
        printf("  Cód: %d - %s (Stock: %d)\n", 
               productos[i].codigo, productos[i].nombre, productos[i].stock);
    }
    
    printf("\nIngrese código del producto a eliminar: ");
    scanf("%d", &codigo);
    eliminarSalto();
    
    // Buscar producto
    int posicion = -1;
    for (int i = 0; i < cantidadProductos; i++) {
        if (productos[i].codigo == codigo) {
            posicion = i;
            break;
        }
    }
    
    if (posicion == -1) {
        printf("No existe producto con código %d\n", codigo);
        return;
    }
    
    printf("\n PRODUCTO ENCONTRADO:\n");
    printf("Código: %d\n", productos[posicion].codigo);
    printf("Nombre: %s\n", productos[posicion].nombre);
    printf("Stock: %d unidades\n", productos[posicion].stock);
    printf("Precio: $%.2f\n", productos[posicion].precio);
    
    char respuesta;
    printf("\n¿Está seguro de eliminar este producto? (s = si/n = no): ");
    scanf(" %c", &respuesta);
    eliminarSalto();
    
    if (respuesta != 's' && respuesta != 'S') {
        printf("Eliminación cancelada.\n");
        return;
    }
    
    for (int i = posicion; i < cantidadProductos - 1; i++) {
        productos[i] = productos[i + 1];
    }
    
    cantidadProductos--;
    
    guardarProductos();
    
    printf(" Producto eliminado exitosamente.\n");
    printf(" Total productos restantes: %d\n", cantidadProductos);
}

void cambiarBajoStock() {
    int nuevoLimite;

    printf("\n--- Cambiar límite de bajo stock ---\n");
    printf("Límite actual: %d\n", limiteStock);

    do {
        printf("Ingrese nuevo límite (>= 0): ");
        scanf("%d", &nuevoLimite);

        if (nuevoLimite < 0) {
            printf("ERROR: El límite debe ser mayor o igual a 0.\n");
        }

    } while (nuevoLimite < 0);

    limiteStock = nuevoLimite;
    printf("Límite actualizado a %d unidades.\n", limiteStock);
    guardaConfiguracion();
}

void mostrarProductos() {
    printf("\n--- LISTA DE PRODUCTOS ---\n");
    printf("==============================================\n");
    printf("%-8s %-20s %-8s %-8s\n", "Código", "Nombre", "Stock", "Precio");
    printf("==============================================\n");
    
    if (cantidadProductos == 0) {
        printf("No hay productos registrados.\n");
        return;
    }
    
    for (int i = 0; i < cantidadProductos; i++) {
        printf("%-8d %-20s %-8d $%-7.2f\n",
               productos[i].codigo,
               productos[i].nombre,
               productos[i].stock,
               productos[i].precio);
    }
    printf("==============================================\n");
    printf("Total de productos: %d\n", cantidadProductos);
}
void redefinirPrecio() {
    int codigoBuscado;
    float nuevoPrecio;
    int encontrado = 0;

    printf("\n--- Cambiar Precio del Producto ---\n");
    printf("Ingrese el código del producto: ");
    scanf("%d", &codigoBuscado);

    // Buscar producto por código
    for (int i = 0; i < cantidadProductos; i++) {
        if (productos[i].codigo == codigoBuscado) {
            printf("Producto encontrado: %s\n", productos[i].nombre);
            printf("Precio actual: %.2f\n", productos[i].precio);

            printf("Nuevo precio: ");
            scanf("%f", &nuevoPrecio);

            productos[i].precio = nuevoPrecio;

            printf("Precio actualizado correctamente.\n");
            encontrado = 1;
            break;
        }
    }

    if (!encontrado) {
        printf("ERROR: No existe un producto con ese código.\n");
    }
}
void guardaConfiguracion() {
    FILE *f = fopen("config.csv", "w"); 
    if (f == NULL) return;

    fprintf(f, "iva,numeroDia,facturaActual,limitestock\n");
    fprintf(f, "%d,%d,%d,%d\n", ivaGlobal, numeroDia, facturaActual, limiteStock);

    fclose(f);
}

void cambiarIVA() {
    int nuevoIVA;

    printf("\n--- Cambiar IVA Global ---\n");
    printf("IVA actual: %d%%\n", ivaGlobal);

    do {
        printf("Ingrese el nuevo IVA (0 - 30): ");
        scanf("%d", &nuevoIVA);

        if (nuevoIVA < 0 || nuevoIVA > 30) {
            printf("ERROR: IVA fuera de rango.\n");
        }

    } while (nuevoIVA < 0 || nuevoIVA > 30);

    ivaGlobal = nuevoIVA;
    guardaConfiguracion();

    printf("IVA actualizado correctamente. Nuevo IVA: %d%%\n", ivaGlobal);
}

void abrirCaja() {
    if (cajaAbierta == 1) {
        printf("La caja ya está abierta.\n");
        return;
    }
    cajaAbierta = 1;
    printf("Caja abierta en el día %d\n", numeroDia);

}


void cerrarCaja() {
    if (!cajaAbierta) {
        printf("La caja ya está cerrada.\n");
        return;
    }

    printf("Cierre del día %d\n", numeroDia);
    cajaAbierta = 0;

    numeroDia++;
    guardaConfiguracion();
}



int cambiarStock() {
    int codigo;
    int idx = -1;
    int nuevostock = 0;
    int i; 
    printf("Ingrese el codigo del producto desea cambiar el stock: ");
    scanf("%d", &codigo) ;
    for (i = 0; i < cantidadProductos; i++)
        if (codigo == productos[i].codigo) {
            printf("\nstock actual: %d", productos[i].stock);
            printf("\nIngrese el nuevo stock del producto: ") ;
            scanf("%d", &nuevostock );
            productos[i].stock =nuevostock ;
            guardarProductos();
            printf("\nStock actualizado. \n");
            idx = i ;
        }

    if (idx == -1){
        printf("producto no encontrado. \n");
    }
    return 0;
}

void mostrarStock() {
    for (int i = 0; i < cantidadProductos; i++ ) {
        printf("El stock de %s es: %d\n", productos[i].nombre, productos[i].stock);
    }
}

void cargarConfig() {
    FILE *f = fopen("config.csv", "r");
    if (f == NULL) return;

    char linea[100];

    fgets(linea, sizeof(linea), f); 
    fgets(linea, sizeof(linea), f);

    char *token = strtok(linea, ",");
    ivaGlobal = atoi(token);

    token = strtok(NULL, ",");
    numeroDia = atoi(token);

    token = strtok(NULL, ",");
    facturaActual = atoi(token);

    token = strtok(NULL, ",");
    limiteStock = atoi(token);

    fclose(f);
}


void eliminarSalto() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void convertirMayusculas(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

void buscarProductosPorNombre() {
    if (cantidadProductos == 0) {
        printf("No hay productos registrados.\n");
        return;
    }
    
    char busqueda[100];
    
    printf("\n=== BUSCAR PRODUCTO POR NOMBRE ===\n");
    printf("Ingrese nombre o parte del nombre: ");
    
    fgets(busqueda, sizeof(busqueda), stdin);
    busqueda[strcspn(busqueda, "\n")] = '\0';
    
    convertirMayusculas(busqueda);
    
    printf("\nBuscando: '%s'\n", busqueda);
    printf("======================================\n");
    
    int encontrados = 0;
    
    for (int i = 0; i < cantidadProductos; i++) {
        char nombreUpper[30];
        strcpy(nombreUpper, productos[i].nombre);
        convertirMayusculas(nombreUpper);
        
        // strstr busca una subcadena dentro de otra
        if (strstr(nombreUpper, busqueda) != NULL) {
            printf("\n%d. Código: %d\n", encontrados + 1, productos[i].codigo);
            printf("   Nombre:  %s\n", productos[i].nombre);
            printf("   Stock:   %d unidades\n", productos[i].stock);
            printf("   Precio:  $%.2f\n", productos[i].precio);
            encontrados++;
        }
    }
    
    if (encontrados == 0) {
        printf("\nNo se encontraron productos con '%s'\n", busqueda);
    } else {
        printf("\n======================================\n");
        printf(" Total encontrados: %d producto(s)\n", encontrados);
    }
}

void buscarProductosPorCodigo() {
    if (cantidadProductos == 0) {
        printf("No hay productos registrados.\n");
        return;
    }
    
    int codigo;
    char buffer[20];
    
    printf("\n=== BUSCAR POR CÓDIGO ===\n");
    printf("Ingrese código del producto: ");
    
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    codigo = atoi(buffer);
    
    int encontrado = -1;
    
    for (int i = 0; i < cantidadProductos; i++) {
        if (productos[i].codigo == codigo) {
            encontrado = i;
            break;
        }
    }
    
    if (encontrado != -1) {
        printf("\n PRODUCTO ENCONTRADO:\n");
        printf("   Código:  %d\n", productos[encontrado].codigo);
        printf("   Nombre:  %s\n", productos[encontrado].nombre);
        printf("   Stock:   %d unidades\n", productos[encontrado].stock);
        printf("   Precio:  $%.2f\n", productos[encontrado].precio);
    } else {
        printf("\n No se encontró producto con código %d\n", codigo);
    }
}


void ordenarPorCodigo() {

    printf("\n===PRODUCTOS ANTES DE ORDENAR===\n");
    mostrarProductos();

    if (cantidadProductos < 2) {
        printf("No hay suficientes productos para ordenar.\n");
        return;
    }
    int respuesta;
    int intercambiado;
    for (int i = 0; i < cantidadProductos - 1; i++) {
        intercambiado = 0;
        for (int j = 0; j < cantidadProductos - i - 1; j++) {
            if (productos[j].codigo > productos[j+1].codigo) {

                Producto temp = productos[j];
                productos[j] = productos[j+1];
                productos[j+1] = temp;
                intercambiado = 1;
            }
        }

        if (!intercambiado) break;
    }
    
    printf(" Productos ordenados por código (ascendente).\n\n");

    mostrarProductos();

    printf("\n¿Desea guardar el ordenamiento?\n");
    do{
        printf("(si = 1, no = 0):");
        scanf("%d", &respuesta);
        if(respuesta == 1){
            guardarProductos();
            break;

        }else{
            if(respuesta == 0){
                break;
            }
            else{
                printf("Ingrese una opcion valida\n");
            }
        }
    }while(1);
}

void ordenarPorNombre() {

    if (cantidadProductos < 2) {
        printf("No hay suficientes productos para ordenar.\n");
        return;
    }
    
    printf("\n=== PRODUCTOS ANTES DE ORDENAR ===\n");
    mostrarProductos(); 
    
    int intercambiado;
    int respuesta; 
    for (int i = 0; i < cantidadProductos - 1; i++) {
        intercambiado = 0;
        
        for (int j = 0; j < cantidadProductos - i - 1; j++) {
            if (strcmp(productos[j].nombre, productos[j+1].nombre) > 0) {
                Producto temp = productos[j];
                productos[j] = productos[j+1];
                productos[j+1] = temp;
                intercambiado = 1;
            }
        }
        
        if (!intercambiado) break; 
    }
    

    printf("\n PRODUCTOS ORDENADOS POR NOMBRE (A-Z)\n");
    mostrarProductos();
    printf("¿Desea guardar el ordenamiento?\n");
    do{
        printf("(si = 1, no = 0):");
        scanf("%d", &respuesta);
        if(respuesta == 1){
            guardarProductos();
            break;

        }else{
            if(respuesta == 0){
                break;
            }
            else{
                printf("Ingrese una opcion valida\n");
            }
        }
    }while(1);
}

int main() {
    SetConsoleOutputCP(65001);
    cargarConfig();
    cargarProductos();
    int opc, opci, rol;
    char opcion[3];
    char cingresada[12];
    char ci[11] = "1724665730";
    char admincontrasena[15];
    char adminpassword[15]="MDRO.2007";
    char usuariopassword[50]="PUNTOFAST.001";
    char usuariocontrasena[50];
    do{
        printf("\n-----LOGIN-----\n");
        printf("1.Administrador\n");
        printf("2.Vendedor\n");
        printf("3.Salir\n");
        printf("Seleccione una opcion: ");
        fgets(opcion, sizeof(opcion), stdin);
        rol = atoi(opcion);

        if(rol == 1 ){
            do{
                printf("\n-----ADMINISTRADOR-----\n");
                printf("Ingrese su cedula y contraseña\n");
                printf("Ingrese 0 en CI si desea cambiar de usuario\n");
                printf("CI: ");
                fgets(cingresada, sizeof(cingresada), stdin);
                cingresada[strcspn(cingresada, "\n")] = '\0';
                if (atoi(cingresada) == 0)
                {
                    break;
                }
                printf("CONTRASEÑA: ");
                fgets(admincontrasena, sizeof(admincontrasena), stdin);
                admincontrasena[strcspn(admincontrasena, "\n")] = '\0';
                if(strcmp(cingresada,ci) == 0 && strcmp(admincontrasena, adminpassword) == 0){
                    printf("INGRESO CORRECTO A ADMINISTRADOR");
                    
                    do{
                        printf("\n-------BIENVENIDO AL PUNTO DE VENTA PUNTOFAST-------");
                        printf("\n1. Catálogo de productos") ;
                        printf("\n2. Inventario") ;
                        printf("\n3. Reporte") ;
                        printf("\n4. Cerrar sesión") ;
                        printf("\nIngrese la opción a la que desea acceder:") ;
                        scanf("%d", &opc) ;
                        eliminarSalto();
                        switch (opc) {
                            case 1:
                                do {
                                    printf("\n---------Ha ingrsado al catalogo de productos-------") ;
                                    printf("\n----PRODUCTOS----") ;
                                    printf("\n1. Registar productos");
                                    printf("\n2. Eliminar productos");         
                                    printf("\n3. Definir precios");
                                    printf("\n4. Definir porcentaje de iva");
                                    printf("\n5. Mostrar Productos");
                                    printf("\n6. Buscar productos por codigo");
                                    printf("\n7. Buscar productos por nombre");
                                    printf("\n8. Regresar") ;
                                    printf("\nIngrese la opcion a la que desea ingresar:") ;
                                    scanf("%d", &opci) ;
                                    eliminarSalto();
                                    switch (opci) {
                                        case 1:
                                            crearProducto();
                                            break;
                                        case 2:
                                            eliminarProducto();
                                            break;    
                                        case 3:
                                            redefinirPrecio();
                                            break;
                                        case 4:
                                            cambiarIVA();
                                            break;
                                        case 5:
                                            mostrarProductos();
                                            break;
                                        case 6:
                                            buscarProductosPorCodigo();
                                            break;
                                        case 7:
                                            buscarProductosPorNombre();
                                            break;
                                        default :
                                            if(opci != 8)
                                                printf("Opcion no valida, intente de nuevo\n");
                                            break;     
                                    }
                                } while (opci != 8) ;
                                break;
                            case 2:
                                do {
                                    printf("\n-------INVENTARIO-------");
                                    printf("\n1. Aumentar o disminuir stock");
                                    printf("\n2. Mostrar stock de productos");
                                    printf("\n3. Cambiar bajo stock");
                                    printf("\n4. Ordenar productos por codigo(menor a myaor)");
                                    printf("\n5. Ordenar productos por nombre(A-Z)");
                                    printf("\n6. Regresar") ;
                                    printf("\nIngrese la opcion a la que desea ingresar:") ;
                                    scanf("%d", &opci) ;
                                    eliminarSalto();
                                    switch (opci) {
                                        case 1:
                                            cambiarStock() ;
                                            break;
                                        case 2:
                                            mostrarStock();
                                            break;
                                        case 3:
                                            cambiarBajoStock();
                                            break;
                                        case 4:
                                            ordenarPorCodigo();
                                            break;
                                        case 5: 
                                            ordenarPorNombre();
                                            break;    
                                        default:
                                            if (opci !=6 ) {    
                                                printf("Opcion no valida, intente de nuevo\n");
                                                break;
                                            }    
                                        }
                                } while (opci != 6 ) ;        
                                break;       

                            case 3 :
                            do{
                                printf("\n---------Reportes---------\n") ;
                                printf("1. Cantidad de ventas\n");
                                printf("2. Bajo stock\n") ;
                                printf("3. Mostrar factura por ID\n");
                                printf("4. Regresar\n") ;
                                printf("Elija lo opcion que desea: ");
                                scanf("%d", &opci);
                                if (opci == 1) {
                                    printf("\nDías disponibles:\n");
                                    for (int d = 1; d < numeroDia; d++) {
                                        printf("%d. Día %d\n", d, d);
                                    }

                                    int diaElegido;
                                    printf("Ingrese el día que desea ver: ");
                                    scanf("%d", &diaElegido);

                                    reportePorDia(diaElegido);
                                    }
                                else if (opci == 2){
                                    bajoStock();
                                
                                }else if(opci == 3){
                                    int id;
                                    printf("Ingrese el ID de la factura: ");
                                    scanf("%d", &id);
                                    mostrarFacturaPorID(id);
                                }
                            }while(opci != 4) ; 

                                break;
                            default:
                                if (opc != 4){    
                                    printf("Opcion no valida, intente de nuevo\n");
                                    break;
                                }    

                        }

                    }while (opc != 4);

                }else{
                    printf("\nCedula o contraseña incorrecta\nIntente de nuevo\n");
                }
            }while(1);    

        }else if(rol == 2){
            do{
                printf("\n----VENDEDOR----\n");
                printf("Ingrese 0 en contraseña si desea cambiar de usuario\n");
                printf("CONTRASEÑA: ");
                fgets(usuariocontrasena, sizeof(usuariocontrasena), stdin);
                usuariocontrasena[strcspn(usuariocontrasena, "\n")] = '\0';
                
                if(strcmp(usuariocontrasena, "0") == 0){
                    break;
                }   
                if(strcmp(usuariocontrasena,usuariopassword) == 0){
                    printf("INGRESO CORRECTO A VENDEDOR\n");
                    
                    do{
                        printf("\n-------BIENVENIDO AL PUNTO DE VENTA PUNTOFAST-------");
                        printf("\n1. Catálogo de productos") ;
                        printf("\n2. Ventas") ;
                        printf("\n3. Caja") ;
                        printf("\n4. Cerrar sesion") ;
                        printf("\nIngrese la opción a la que desea acceder:") ;
                        scanf("%d", &opc) ;
                        eliminarSalto();
                        switch (opc) {
                            case 1:
                                do {
                                    printf("\n---------Ha ingrsado al catalogo de productos-------") ;
                                    printf("\n----PRODUCTOS----") ;
                                    printf("\n1. Buscar productos por codigo");
                                    printf("\n2. Buscar productos por nombre");
                                    printf("\n3. Mostrar Productos");
                                    printf("\n4. Regresar") ;
                                    printf("\nIngrese la opcion a la que desea ingresar:") ;
                                    scanf("%d", &opci) ;
                                    eliminarSalto();
                                    switch (opci) {
                                        case 1:
                                            buscarProductosPorCodigo();
                                            break;
                                        case 2:
                                            buscarProductosPorNombre();
                                            break;                                        
                                        case 3:
                                            mostrarProductos();
                                            break;
                                        default:
                                            if(opci != 4)
                                                printf("Opcion no valida, intente de nuevo\n");
                                            break;     
                                    }
                                } while (opci != 4) ;
                                break;
                                    
                            case 2:
                                do {
                                    printf("1. Gestionar ventas\n");
                                    printf("2. Regresar\n");
                                    printf("Ingrese la opcion: ");
                                    scanf("%d", &opci);

                                    if (opci == 1){
                                        venderCarrito();

                                    }else if (opci == 2){
                                        break;
                                        
                                    }else printf("Error ingrese una opcion valida\n");    
                                    
                                } while (opci != 2);

                                break;
                                    
                            case 3:
                                do{
                                    printf("\n-------CAJA-------");
                                    printf("\n1. Abrir caja");
                                    printf("\n2. Cerrar caja");
                                    printf("\n3. Regresar") ;
                                    printf("\nOpción: ");
                                    scanf("%d", &opci);
                                    
                                    if (opci == 1) {
                                        abrirCaja();
                                    }
                                    else if (opci == 2) {
                                        cerrarCaja();
                                    }    
                                    else {
                                        if (opci != 3)
                                        {
                                            printf("Opcion no valida\n");
                                        }
                                        
                                    }
                                    break;
                                } while(opci != 3) ;   
                                break;

                            
                            default:
                                if (opc != 4){    
                                    printf("Opcion no valida, intente de nuevo\n");
                                    break;
                                }    

                        }

                    }while (opc != 4);

                }else{
                    printf("\nContraseña incorrecta\nIntente de nuevo\n");
                }
            }while(1);    
        }else{
            if(rol == 3){
                continue;
            }
            else{
                printf("ERROR Ingrese una opcion valida");
            }
        }

        
    }while(rol != 3);
    
}

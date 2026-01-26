PuntoFast - Sistema de Punto de Venta
Descripción del Proyecto
PuntoFast es un sistema de punto de venta desarrollado en lenguaje C para la gestión de inventario, ventas y reportes de un establecimiento comercial. 
El sistema cuenta con diferentes niveles de acceso (administrador y vendedor) y funcionalidades robustas para la gestión de productos, facturación e inventario.

Características Principales
🔐 Sistema de Autenticación
Administrador: Acceso completo a todas las funcionalidades del sistema

Vendedor: Funciones limitadas a ventas y consulta de productos

📦 Gestión de Productos
Crear, eliminar y modificar productos

Asignar códigos automáticamente

Definir precios y stock

Búsqueda por código o nombre

Ordenamiento por código o nombre

💰 Módulo de Ventas
Proceso de venta con múltiples productos

Cálculo automático de IVA configurable

Generación de facturas con ID único

Gestión de carrito de compras

Control de stock en tiempo real

📊 Inventario y Reportes
Control de stock con alertas de bajo inventario

Reportes de ventas por día

Consulta de facturas por ID

Configuración de límite de stock mínimo

💼 Gestión de Caja
Apertura y cierre de caja diario

Numeración automática de días

Persistencia de datos entre sesiones

Estructura del Proyecto
Archivos Principales
text
puntofast.c      
productos.csv           
facturas.csv            
config.csv              
===ESTRUCTURA DE DATOS===
Producto: código, nombre, stock, precio

FacturaDetalle: información completa de ventas

ItemCarrito: productos en proceso de venta

ResumenProducto: datos para reportes

Requisitos del Sistema
Software
Compilador C (GCC, MinGW, etc.)

Sistema operativo Windows

Terminal con soporte UTF-8

Librerías Utilizadas
c
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <string.h>
===DATOS PARA EL USO===
puntofast.exe
Credenciales de Acceso
Administrador
Cédula: 1724665730

Contraseña: MDRO.2007

Vendedor
Contraseña: PUNTOFAST.001

Funcionalidades por Rol
👨‍💼 Administrador
Catálogo completo de productos

Gestión de inventario

Reportes detallados

Configuración del sistema

Cambio de IVA y límites de stock

👨‍💼 Vendedor
Consulta de productos

Proceso de ventas

Apertura/cierre de caja

No puede modificar precios ni stock

=====FUNCIONES=====

eliminarProducto(): Elimina productos del sistema

redefinirPrecio(): Modifica precios

mostrarProductos(): Lista todos los productos

Ventas
venderCarrito(): Proceso completo de venta

guardarFactura(): Almacena datos de venta

mostrarFacturaPorID(): Consulta facturas específicas

Inventario
cambiarStock(): Modifica cantidad en inventario

bajoStock(): Muestra alertas de inventario bajo

ordenarPorCodigo(): Ordena productos

ordenarPorNombre(): Ordena alfabéticamente

======Configuración======
cambiarIVA(): Modifica porcentaje de IVA

cambiarBajoStock(): Define límite de stock mínimo

abrirCaja(): Inicia jornada comercial

cerrarCaja(): Finaliza día de ventas

Consideraciones Técnicas
Persistencia de Datos
Todos los datos se guardan en archivos CSV

Configuración se mantiene entre ejecuciones

Sistema numérico automático para facturas y días

Validaciones
Control de stock insuficiente

Verificación de caja abierta para ventas

Validación de credenciales

Rango de valores para IVA (0-30%)

Interfaz de Usuario
Menús jerárquicos organizados

Mensajes de error descriptivos

Formato de salida claro y legible

====Limitaciones Conocidas=====
Capacidad máxima: 100 productos y 50 ítems por carrito

Requiere reinicio para aplicar algunos cambios de configuración

Solo funciona en sistemas Windows por el uso de windows.h


Licencia
Este proyecto NO TIENE LICENCIA actualmente

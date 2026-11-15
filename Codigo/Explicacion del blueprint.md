# Código y escenario de la práctica 4.1

## Modelo económico del Arduino
Los potenciómetros se leen con `analogRead` (0 a 1023) y se convierten con `map` a una escala de 10 a 90:

- `demanda` = potenciómetro en A0
- `oferta` = potenciómetro en A1

Modelo lineal: la demanda baja con la cantidad (`P = a − Q`) y la oferta sube con la cantidad (`P = Q − b`), con `a = 100 + demanda` y `b = oferta − 50`. Al igualar ambas, el precio de equilibrio es `(a − b) / 2`, que el programa multiplica por 4 para expresarlo en pesos por kilo (división entera en C++).

Ejemplo de la práctica: demanda 80 y oferta 20 → a = 180, b = −30, precio = ((180 + 30) / 2) × 4 = **420**.

La matriz LED muestra el precio con tres dígitos dibujados a mano (sin la biblioteca ArduinoGraphics) y enciende un punto en la esquina cuando hay escasez.

Si `demanda − oferta ≥ 25` el programa considera que hay **escasez** y envía una alerta, pero solo cuando el mercado *pasa* de normal a escasez y han pasado más de 60 segundos desde la última alerta (así no se gastan créditos de Make en cada lectura).

## Comunicación con Make
`enviarAMake` abre una conexión segura (`WiFiSSLClient`, puerto 443) al dominio del webhook y envía una petición `GET` con los datos en la URL: `?demanda=80&oferta=20&precio=420&estado=escasez`.

## Escenario de Make `Mercado_Equipo`
| # | Módulo | Qué hace |
|---|--------|----------|
| 1 | **Webhooks → Custom webhook** | Recibe `demanda`, `oferta`, `precio` y `estado` del Arduino. |
| — | **Filtro** (entre 1 y 2) | `Solo si hay escasez`: deja pasar el mensaje únicamente si `{{1.estado}}` es igual a `escasez`. |
| 2 | **Telegram Bot → Send a Text Message or a Reply** | Envía al Chat ID configurado el texto: `ALERTA DE ESCASEZ`, demanda, oferta, precio de equilibrio por kg y la explicación. |

## Importar
En Make: *Create a new scenario → ⋯ → Import Blueprint*, seleccionar `blueprint.json`, crear un webhook nuevo en el módulo 1, elegir tu conexión del bot de Telegram en el módulo 2, escribir tu Chat ID y activar el escenario. Después, en el `.ino`, cambiar `RUTA_MAKE` por la ruta de tu webhook y escribir el nombre y la clave de tu Wi-Fi.

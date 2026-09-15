# TP de Sistemas Operativos 
UTN FRBA  -  2do Cuatrimestre - 2025

## Participantes
* Elias Nicolas Aires
* Sofia Mei Sakugawa
* Eduardo Andres Gaidano Riquel 

## Dependencias

Para poder compilar y ejecutar el proyecto, es necesario tener instalada la
biblioteca [so-commons-library] de la cátedra:

```bash
git clone https://github.com/sisoputnfrba/so-commons-library
cd so-commons-library
make debug
make install
```

Además es recomendable utilizar la Máquina Virtual de la cátedra, ya que el trabajo fue desarrollado para ser capaz de ejecutarse en ella.


Si se prefiere trabajar en otro entorno Linux, es recomendable verificar los archivos `.config` de cada módulo.


Maquinas Virtuales: https://docs.utnso.com.ar/recursos/vms.html


## Compilación y ejecución

Cada módulo del proyecto se compila de forma independiente a través de un
archivo `makefile`. Para compilar un módulo, es necesario ejecutar el comando
`make` desde la carpeta correspondiente.

El ejecutable resultante de la compilación se guardará en la carpeta `bin` del
módulo. Ejemplo:

```sh
cd master
make
./bin/master
```

## Documentos

- [Enunciado del Trabajo Práctico](https://docs.google.com/document/d/10mDZOvxNkfaYL_jtYYsdH6m3uxHlvPGzHIt1irRGYcw/edit?usp=sharing)
- [Pruebas Finales](https://docs.google.com/document/d/16uSiHv0Of6uaG_oE5FtI3oWVLsAfEAwS5v3To8FY_xg/edit?usp=sharing)
- [Resultados de Pruebas](https://docs.google.com/document/d/1EBQ-eZbmp3YxU4pMrkX1dPcFKzpaiw1ugcQiisrj8o0/edit?usp=sharing)

[so-commons-library]: https://github.com/sisoputnfrba/so-commons-library

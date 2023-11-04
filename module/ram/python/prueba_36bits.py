import math

def procesar_archivo(archivo_entrada, archivo_salida):
    with open(archivo_entrada, 'r') as entrada, open(archivo_salida, 'w') as salida:
        counter = 0
        lineas_entrada = entrada.readlines()
        total_lineas = len(lineas_entrada)
        a=int(total_lineas/4)
        for num_linea in range(3200):
            line1 = lineas_entrada[num_linea]
            line2 = lineas_entrada[num_linea + 3200]
            line3 = lineas_entrada[num_linea + 6400]
            line4 = lineas_entrada[num_linea + 9600]
            lines = [line1,line2,line3,line4]
            for e in range(len(lines)):
                num = obtener_numeros(lines[e])
                for i in range(3):
                    bin = binarizar(num[i])
                    bin = bin.zfill(3)
                    salida.write(bin)
            salida.write('\n')


def obtener_numeros(linea):
    inicio = linea.index('(') + 1
    fin = linea.index(')')
    numeros_str = linea[inicio:fin].split(',')
    numeros = [int(int(numero)/34) for numero in numeros_str]
    return numeros

def binarizar(decimal):
    binario = ''
    while decimal // 2 != 0:
        binario = str(decimal % 2) + binario
        decimal = decimal // 2
    return str(decimal) + binario

archivo_entrada = 'prueba.txt'
archivo_salida = 'ext_mem.txt'

procesar_archivo(archivo_entrada, archivo_salida)




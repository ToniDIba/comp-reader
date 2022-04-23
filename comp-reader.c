/********************************************************
 *                                                                          
 * Filename: comp_reader.c                                                        
 *                                                                          
 * Purpose : Lee fichero .BIN                  
 *                                                                          
 * History : Creado: 22.04.2006                                             
 *           02-09-10  Created         
 *       DEBUG = F11
 *                                                                          
 /*******************************************************/

// SORTIPRO2.BIN 0 16 128
// GHBO048.BIN 0 1000 130000 113
// SALUD.BIN 0 203 5146253 57
// SORTIDA-OK.BIN 0 64 64 20
// ULTIM.BIN 0 103 103 22
// RFPROVA2-2.BIN 0 50 5000000 12
// RFPROVA2-2.BIN 90000 50 5000000 12 NOWHERE
// GHBO048.BIN 0 1000 130000 113 #19:VALENCIA#


#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <conio.h>
#include <limits.h>
#include <stddef.h>



int main(int argc, char *argv[]);
char nombre_bin_aux[30];
char path_archivo_bin[30];
char path_archivo_plant[30];
char path_archivo_regs[30];




void cerrar_archivos(void);
void cerrar_archivo_xls(void);
void llenar_archivo_regs(void);
void cerrar_archivo_plant(void);
void cerrar_archivo_regs(void);
void procesos_finales(void);
void presentacion(void);
void comprueba_ESC_pulsado(void);


void recoger_parametros(char *argv[]);
void imprimir_parametros(int argc, char *argv[]);
void limpiar_campos(unsigned char datos_leidos[], char todo[]);
void limpiar_arrays(char tiene_signo[], char tipo[], int num_dec[], int ocupa_host[], int ocupa_editat[], int pos_ini[]);
void informa_arrays(char tiene_signo[], char tipo[], int num_dec[], int ocupa_host[], int ocupa_editat[], int pos_ini[]);
void trata_numerico(long longi, char signo[], int num_decs, char linea_registro[]);
void trata_alfanumerico(long longi, long longi_edit, char linea_registro[]);
void trata_comp3(long longi, char signo[], int num_decs, char linea_registro[]);
void mi_dictionary(char valor_a_buscar[], char valor_encontrado[]);
void trata_comp(long longi, char linea_registro[]);
void quita_ceros(char total[], char total_net[]);


int comprobar_parametros(int argc);
int crear_archivo_regs(void);
int crear_archivo_aviso(void);
int crear_archivo_salida(void);
int escribir_aviso(char aviso[]);
int abrir_archivo_bin(void);
int mira_si_salgo(void);
int graba_linea_registro(char linea_registro[]);
int abrir_archivo_regs(void);
int abrir_archivo_plantilla(void);
int lee_datos_plantilla(void);   
int tratar_archivo_regs(void);
int hextodec(const char *hex);
int extrae_valor(int num, char extraido[], char donde_busco[]);


long extrae_numero(char leido[], char extraido_str[]);
long num_regs_cumplen = 0;
long longitud(char []);



//-------- Paràmetres de entrada ---------------------------
char nombre_BIN[30];
long regs_a_omitir;
long longitut_copy;
long longitut_arxiu_bin;
int lineas_copy;
char where[80];
//----------------------------------------------------------


FILE *arxiuBin;  // Arxiu .BIN
FILE *arxiuPlant;  // Arxiu de plantilla
FILE *arxiuRegs; // n regs. del arxiu entrada .BIN
FILE *arxiuAvis; // n regs. del arxiu entrada .BIN
FILE *arxiuXls; // n regs. del arxiu entrada .BIN


int main(argc,argv) int argc; char *argv[];
{

	presentacion();              // Muestra cabecera 
	comprobar_parametros(argc);  // Comprueba que llegue el nº de parámetros esperado
	recoger_parametros(argv);    // Asigna paràmetros de entrada a las variables
	crear_archivo_aviso();       // Para que Excel sepa que C ha terminado
	abrir_archivo_bin();         // Abre archivo .BIN
	crear_archivo_regs();        // Crea archivo auxiliar donde dejar registros del .BIN
	llenar_archivo_regs();       // Crea n registros a partir del archivo de entrada
	cerrar_archivos();
	tratar_archivo_regs();       // Formatea registro segun archivo plantilla
	procesos_finales();
    exit(0);
}


void presentacion(void)
{

	_clrscr();
	_gotoxy(3,0);
	printf("\n");
	printf("*****************************************************************************\n");
	printf("*                                                                           *\n");
	printf("*         C O M P  R E A D E R  F A S T !                                   *\n");
	printf("*                                                                           *\n");
	printf("*****************************************************************************\n");
	printf("\n");
	printf("Subrutina escrita en lenguaje \"C\" para la conversion de registros HOST EBCDIC\n");
	printf("en registros ASCII.\n\n");


}


int comprobar_parametros(argc) int argc;
{
	char mensaje[90];
	memset(mensaje,'\0',80);

	if(argc != 7) 
	{
		strcpy(mensaje, "Error! Faltan parametros en llamada al C\0\0");
	    escribir_aviso(mensaje); // Deja mensaje a Excel 
		printf("\nFaltan parametros !\n");
		printf("%s %d", "Num params: " , argc);
		printf("\n");
	 	exit(1);
    }

	return 0;
}

void procesos_finales(void)
{
	char mensaje[90];
	char en_string[40];
	long regs_a_tractar;

	printf("\n\n");
	regs_a_tractar = (long) ( longitut_arxiu_bin / longitut_copy );
	if(regs_a_omitir > 0 ) regs_a_tractar = (long) regs_a_tractar - regs_a_omitir;
	memset(en_string,'\0',35);
	memset(mensaje,'\0',80);
	if(! strcmp(where, "NOWHERE")) // No han puesto where
	{
		_ultoa(regs_a_tractar, en_string,10);
		puts(en_string);
	}
	else
	{
		_ultoa(num_regs_cumplen, en_string,10);
	}
	strcpy(mensaje, "Ya he terminado. Regs = \0");
	strcat(mensaje, en_string);
	strcat(mensaje,"\0\0");
	escribir_aviso(mensaje); // Deja mensaje a Excel para que sepa que ya ha terminado

}


void recoger_parametros(argv) char *argv[];
{
	// Parametres entrada:  0 - Nom del executable ( no usat )
    //                      1 - Nom del .BIN
    //                      2 - Num. regs. a omitir
    //                      3 - Longitut de la Copy
	//                      4 - Longitut del arxiu
	//                      5 - Num lineas copy
	//                      6 - Condiciones where
	                        
	sprintf(nombre_BIN,argv[1]);
	regs_a_omitir = atol(argv[2]);
	longitut_copy = atol(argv[3]);
	longitut_arxiu_bin = atol(argv[4]);
	lineas_copy = atoi(argv[5]);
	sprintf(where,argv[6]);

	
	
	/*printf("\n%s%s"  ,"Nom .Bin     : ", nombre_BIN);
	printf("\n%s%ld" ,"Omitir       : ", regs_a_omitir);
	printf("\n%s%ld" ,"Long copy    : ", longitut_copy);
	printf("\n%s%ld" ,"Long arxiu   : ", longitut_arxiu_bin);
	printf("\n%s%d"  ,"Linias copy  : ", lineas_copy);
	printf("\n%s%s\n","Where        : ", where);

	getchar(); */
	

}



void llenar_archivo_regs(void)
{
	long numRegs;    // Según la copy, el nº de registros que existen en el .BIN
	long desplBin;   // Offset del .BIN
	long desplRegs;  // Offset del archivo auxiliar con los registros a tratar
	char en_hexa[] = "  ";
	char mensaje[90];
	char en_string[40];
	int k;
	long i;
	long tempo = 0;
	long cuantos = 0;
	
	unsigned char *datosLeidos; // Crea un array dinàmic
	datosLeidos = (unsigned char *)malloc(longitut_copy);
	
	int lon = longitut_copy * 2;
	char str1[lon];
	char todo[lon];


	// -------------------------------------------------------------------------------------------


	numRegs = longitut_arxiu_bin / longitut_copy;
	printf("%s %ld %s", "Registros a tratar:",numRegs,"\n\n");
	printf("----------------------------------------------------------------------------");

	fseek(arxiuBin,0l,0);
	desplBin = 0;
	desplRegs = 0;
	str1[0] = '\0';

    if(regs_a_omitir > 0)
	{
		for(i=1; i <= regs_a_omitir; i++)
		{
			fseek(arxiuBin,desplBin,0);
			fread(datosLeidos, (longitut_copy * sizeof(unsigned char)) , 1 , arxiuBin);
		    desplBin = desplBin + longitut_copy;
		}

	}


	for(i = (regs_a_omitir + 1) ; i <= numRegs; i++)
	{	
		limpiar_campos(datosLeidos, todo); 
		fseek(arxiuBin,desplBin,0);
		fread(datosLeidos, (longitut_copy * sizeof(unsigned char)) , 1 , arxiuBin);

		for(k = 0; k <= (longitut_copy - 1); k++ )
		{
			sprintf(en_hexa,"%02X",datosLeidos[k]);
			strcat(str1, en_hexa);
			strcat(str1,"\0\0");
		}

		strcat(todo,str1);
		strcat(todo,"\0\0");
		str1[0] = '\0';
		fseek(arxiuRegs,desplRegs,0);
	    fwrite(&todo, (longitut_copy * sizeof(char)*2),1,arxiuRegs);
		tempo++;
		cuantos++;

		
		if(tempo == 5000)
		{
		   comprueba_ESC_pulsado(); // Mira si han pulsado la tecla ESC
		   tempo = 0;
		   _gotoxy(01,20);
		   printf("%s %ld","Grabados: ", cuantos);
		   memset(en_string,'\0',35);
		   _ultoa(cuantos, en_string,10);
		   memset(mensaje,'\0',80);
		   strcat(mensaje, "Tratados paso 1: \0");
		   strcat(mensaje, en_string);
		   strcat(mensaje,"\0\0");
		   escribir_aviso(mensaje);
		}

		todo[0] = '\0';
		desplBin = desplBin + longitut_copy;
        desplRegs = desplRegs + longitut_copy * 2;
		
	}

}




//---------------------------------------------------------------------------------------

// Si han pulsado tecla ESC se lo hago saber a Excel
void comprueba_ESC_pulsado(void)
{
	int tecla;
	char mensaje[90];

	tecla = _kbhit();

	if(tecla != 0) // Algo han pulsado . . . 
	{
		tecla = _getch();
		if(tecla == 27) // 27 = Esc
		{
			memset(mensaje,'\0',80);
			strcat(mensaje, "Han pulsado ESC.\0");
			escribir_aviso(mensaje);
			exit(0);
		}

	}

}




//Llegeix registre a registe arxiu registres i formateja les dades segons dicti arxiu plantilla
int tratar_archivo_regs(void)
{
	
	long regs_a_tractar;
	long i,k,longi,longi_edit;
	long cuantos = 0;
	long aux = 0;
	long ik;
	int num_decs;

	char signo[4];
	char mensaje[90];
	char en_string[40];
	char patron[10];
	char carac2;
	char valor_where[1000];
	char valor_registro[1000];
    char cumplidoras[2000];
	

	// Donde guardo contenido archivo "plantilla"
	char tiene_signo[lineas_copy];
	char tipo[lineas_copy];
	int num_dec[lineas_copy];
	int ocupa_host[lineas_copy];
	int ocupa_editat[lineas_copy];
	int pos_ini[lineas_copy];
	char linea_registro[longitut_copy + lineas_copy]; // + ';' per l'excel


	regs_a_tractar = (long) ( longitut_arxiu_bin / longitut_copy );

	if(regs_a_omitir > 0)
	{
		regs_a_tractar = regs_a_tractar - regs_a_omitir;
	}

	abrir_archivo_plantilla();   // Abre archivo con la definición del registro
	abrir_archivo_regs();        // Abre archivo con los registros a tratar

	limpiar_arrays(tiene_signo, tipo, num_dec, ocupa_host, ocupa_editat, pos_ini); // Inicialitza arrays
	informa_arrays(tiene_signo, tipo, num_dec, ocupa_host, ocupa_editat, pos_ini); // Plantilla >> arrays

	crear_archivo_salida();
	abrir_archivo_regs();
	fseek(arxiuRegs,0l,0);
	printf("%s %ld %s","Registres a tractar:" , regs_a_tractar, "\n");


	for(ik = 0; ik <= regs_a_tractar - 1; ik++)  // Trato registro
	{

		aux++;
		cuantos++;

		if(aux == 5000)
		{
		   comprueba_ESC_pulsado(); // Mira si han pulsado la tecla ESC
		   aux = 0;
			_gotoxy(01,21);
		   printf("%s %ld","Tratados: ", cuantos);
		   memset(en_string,'\0',35);
		   _ultoa(cuantos, en_string,10);
		   memset(mensaje,'\0',80);
		   strcat(mensaje, "Tratados paso 2: \0");
		   strcat(mensaje, en_string);
		   strcat(mensaje,"\0\0");
		   escribir_aviso(mensaje);
		}

		memset(linea_registro,'\0',(longitut_copy + lineas_copy));

		for(k = 0; k <= lineas_copy - 1; k++) // Dentro de cada registro, variable a variable
		{

			/*printf("%s %c %s", "Signo  :", tiene_signo[k],"\n");
			printf("%s %c %s", "Tipo   :", tipo[k],"\n");
			printf("%s %d %s", "Num dec:", num_dec[k],"\n");
			printf("%s %d %s", "Ocupa Host  :", ocupa_host[k],"\n");
			printf("%s %d %s", "Ocupa editat:", ocupa_editat[k],"\n");
			printf("%s %d %s", "Pos ini:", pos_ini[k],"\n"); */


			longi = ocupa_host[k];
			longi_edit = ocupa_editat[k];
			signo[0] = tiene_signo[k];
			signo[1] = '\0';
			num_decs = num_dec[k];

			switch (tipo[k]) {   

				case 'N': trata_numerico(longi, signo, num_decs, linea_registro); break;
   				case 'A': trata_alfanumerico(longi, longi_edit, linea_registro); break;
    			case '3': trata_comp3(longi, signo, num_decs, linea_registro); break;
    			case 'C': trata_comp(longi, linea_registro); break;
    			default: puts("ni flowers. Comprueba plantilla.txt"); getchar; break;

			}

		}


		if ( strcmp(where, "NOWHERE")  ) // Si Sí han puesto where . . .
		{
			memset(cumplidoras,'\0',1950);

			for(i = 0; i <= lineas_copy; i++)
			{
				char enchar[10];
				char *p;

				memset(patron,'\0',8);
				memset(enchar,'\0',8);
				int num = i + 1;
				_itoa(num,enchar,10);

				strcat(patron,"#");
				strcat(patron,enchar);
				strcat(patron,":");
	
				p = strstr(where,patron);
			
				if(p == NULL)
				{
				
				}
				else
				{

					memset(valor_where,'\0',950);
					int m = 0;
					carac2 = '\0';
	
					for(int k = 1; carac2 != '#' ; k++)
					{
						carac2 =  _toascii(p[k]);
						if(carac2 == ':') // 58 es el ASCII de ':'
						{
							for(int h = (k + 1); carac2 != '#' ; h++)
							{
						       carac2 =  _toascii(p[h]);
					 	       if(carac2 != '#')  valor_where[m++] = carac2; 
							}
				   		}
			    	}

     	            memset(valor_registro,'\0',950);
					extrae_valor(num, valor_registro, linea_registro);

					if ( ! strcmp(valor_where,valor_registro))
					{
						strcat(cumplidoras,patron);
						strcat(cumplidoras,valor_registro);
						strcat(cumplidoras,"\0\0");
					}
	
				}

			}

			strcat(cumplidoras,"#\0\0");

		}

		if ( (! strcmp(where,cumplidoras)) || (! strcmp(where, "NOWHERE")) )
		{
			memset(cumplidoras,'\0',1950);
			graba_linea_registro(linea_registro);
			num_regs_cumplen++;
		}

	}

	cerrar_archivo_regs();
	cerrar_archivo_xls();
	return 0;


}


// Extrae un substring de un string
int extrae_valor(int num, char extraido[], char donde_busco[])
{
	int num_buscado = num;
	int found;
	int longit;
	char carac;
	char carac2;
	int m=0;
	int espacios;
	char valor_where[1000];


	found = 0;
	longit = strlen(donde_busco);
	memset(valor_where,'\0',950);
	carac2 = '\0';

	for(int i = 0; i <= longit; i++)
	{
		if(num_buscado == 1)
		{
			for(int k = 0; carac2 != ';' ; k++)
			{
			    carac2 =  _toascii(donde_busco[k]);
				if(carac2 != ';') valor_where[m++] = carac2; // 59 es el ASCII de ';'
			}

			strcpy(extraido,valor_where);
			return 0;
		}
		else
		{
			carac = donde_busco[i];
			if(carac == ';') found++;

			if(num_buscado - found == 1)
			{
		 	    m = 0;
				for(int k = (i + 1); carac2 != ';' ; k++)
				{
			 	   carac2 =  _toascii(donde_busco[k]);
				   if(carac2 != ';') valor_where[m++] = carac2; // 59 es el ASCII de ';'
				}

				_strrev(valor_where);
				for(int i=0; i <= strlen(valor_where);i++)
				{
					if(valor_where[i] == ' ')
					{
 						espacios++;
					}
					else
					{	
						_strrev(valor_where);
						int info = strlen(valor_where) - espacios;
                        valor_where[info] = '\0';
						break;
					}

				}

				strcpy(extraido,valor_where);
				return 0;
	 		}
		}
	}


	return 0;
	
}

//-----------------------------------------------------------------------------------------------
// PIC S9(9)COMP.
//-----------------------------------------------------------------------------------------------
void trata_comp(long longi, char linea_registro[])
{
	char leido[50];
	char enchar[50];
	char *end;
	long longo;


	memset(leido,'\0',40);
	memset(enchar,'\0',40);
	
	fread(&leido, ( longi * sizeof(char)) * 2 ,1,arxiuRegs);
	fseek(arxiuRegs,0l,1);
	longo = strtol(leido, &end, 16);
	_ultoa(longo,enchar,10);

	strcat(enchar,";\0");
	strcat(linea_registro,enchar);

}

//-----------------------------------------------------------------------------------------------
// PIC S9(9)V99.
//-----------------------------------------------------------------------------------------------
void trata_numerico(long longi, char signo[], int num_decs, char linea_registro[])
{
	char leido[1500];
	char extraido_str[50];
	char total[25];
	char total_net[25];
	char parte_entera[20];
	char parte_decimal[20];
	char aux5[30];
	long numero_extraido;
	char penultima_letra;
	int pos_ini;
	int entera;
	int longnum;
	int lon, k;


	memset(leido,'\0',1490);
	fread(&leido, ( longi * sizeof(char)) * 2 ,1,arxiuRegs);
	fseek(arxiuRegs,0l,1);
	numero_extraido = extrae_numero(leido, extraido_str);
	longnum = strlen(extraido_str);
	
	if(!strcmp(signo,"S")) //Si tiene signo
	{
		lon = strlen(leido) - 2;
		penultima_letra = leido[lon];
        if(penultima_letra == 'D') // Indica negativo.
		{
			numero_extraido = numero_extraido * -1;
			memset(aux5,'\0',20);
			aux5[0] = '-';
			strcat(aux5,extraido_str);
			memset(extraido_str,'\0',40);
			strcpy(extraido_str, aux5);
		}

	}

	// Si tiene decimales . . .
	if(num_decs > 0)
	{

		// Extraer parte entera -----------------------------------------------
		if(longnum > num_decs) // Quiere decir que tiene parte entera 
		{
			memset(parte_entera,'\0',15);
			if(numero_extraido < 0)
			{
				entera = (longnum - num_decs) + 1; // +1 por el signo '-'
                strncpy(parte_entera,extraido_str,entera);
			}
			else
			{
			    entera = (longnum - num_decs);
                strncpy(parte_entera,extraido_str,entera);
			}	
		}
		else
		{
			strcpy(parte_entera,"0\0");
		}

        // Extraer parte decimal ---------------------------------------------------
	
        pos_ini = entera;
	
		memset(parte_decimal,'\0',15);
		for(k = 0; k <= num_decs; k++)
		{
			parte_decimal[k] = extraido_str[pos_ini];
			pos_ini = pos_ini + 1;
		}

		// Convierte el 3 de 12,3 en 30 ( 12,30 )
        if( strlen(parte_decimal) < num_decs)
		{
            for(k = 0; k <= (num_decs - strlen(parte_decimal)); k++ ) strcat(parte_decimal,"0\0");
	    }

		strcat(parte_entera,",\0");
		memset(total,'\0',20);
		strcpy(total,parte_entera);
		strcat(total,parte_decimal);
		strcat(total,"\0");

		quita_ceros(total, total_net); // Elimina 000 de 000234,5
     
	}
	else // No tiene decimales
	{
		memset(total,'\0',20);
		memset(total_net,'\0',20);
		strcpy(total_net,"0\0");

		if(numero_extraido != 0) quita_ceros(extraido_str, total_net);

	}

	strcat(total_net,";\0");
	strcat(linea_registro,total_net);

}


//---------------------------------------------------------------------------------------------
// PIC X(50).
//---------------------------------------------------------------------------------------------
void trata_alfanumerico(long longi, long longi_edit, char linea_registro[])
{
	char leido[1500];
	char valor_a_buscar[5];
	char valor_encontrado[6];
	int lonx, i;
	char aux[250];
	char nulo[40];

	memset(leido,'\0',1490);
	fread(&leido, ( longi * sizeof(char)) * 2 ,1,arxiuRegs);
	fseek(arxiuRegs,0l,1);

	lonx = strlen(leido);
	memset(aux,'\0',248);
	valor_a_buscar[2] = '\0';

	for(i = 0; i < lonx; i += 2)
	{
		valor_a_buscar[0] = leido[i];
		valor_a_buscar[1] = leido[i+1];
		mi_dictionary(valor_a_buscar, valor_encontrado);
		strcat(aux,valor_encontrado);
	}

	memset(nulo,'\0',30);
	strncpy(nulo,aux,4);

	if(!strcmp(nulo,"NULL"))
	{
		memset(aux,'\0',40);
		strcpy(aux,"?\0");
	}
	
	strcat(aux,";\0");
	if(strlen(aux) == 0 )
	{
		puts("Longitut 0");
		getchar();
	}

	strcat(linea_registro,aux);

}



// PIC S9(09)V99 COMP-3.
void trata_comp3(long longi, char signo[], int num_decs, char linea_registro[])
{
	
	
	char leido[50];
	char sense_lletra[50];
	char aux5[30];
	char total[50];
	char total_net[50];
	char parte_entera[20];
	char parte_decimal[20];
	char ultima_letra;
	int entera;
	long long ennum;
	int pos_ini,k;

	
	int longit;

	memset(leido,'\0',40);
	fread(&leido, ( longi * sizeof(char)) * 2 ,1,arxiuRegs);
	fseek(arxiuRegs,0l,1);
	longit = strlen(leido) - 1;
	ultima_letra = leido[longit];

	memset(sense_lletra,'\0',40);
	strncpy(sense_lletra,leido,longit);
	ennum = atoll(sense_lletra);

	if(!strcmp(signo,"S")) //Si tiene signo
	{
		longit = strlen(leido) - 2;
        if(ultima_letra == 'D') // Indica negativo.
		{
			ennum = ennum * -1;
			memset(aux5,'\0',20);
			aux5[0] = '-';
			strcat(aux5,sense_lletra);
		}

	}

	longit = strlen(sense_lletra);

	// Si tiene decimales . . .
	if(num_decs > 0)
	{

		// Extraer parte entera -----------------------------------------------
		if(longit > num_decs) // Quiere decir que tiene parte entera 
		{
			memset(parte_entera,'\0',15);
			if(ennum < 0)
			{
				entera = (longit - num_decs); // +1 por el signo '-'
                strncpy(parte_entera,sense_lletra,entera);
			}
			else
			{
			    entera = (longit - num_decs);
                strncpy(parte_entera,sense_lletra,entera);
			}	
		}
		else
		{
			strcpy(parte_entera,"0\0");
		}

        // Extraer parte decimal ---------------------------------------------------
	
        pos_ini = entera;
	
		memset(parte_decimal,'\0',15);
		for(k = 0; k <= num_decs; k++)
		{
			parte_decimal[k] = sense_lletra[pos_ini];
			pos_ini = pos_ini + 1;
		}

		// Convierte el 3 de 12,3 en 30 ( 12,30 )
        if( strlen(parte_decimal) < num_decs)
		{
            for(k = 0; k <= (num_decs - strlen(parte_decimal)); k++ ) strcat(parte_decimal,"0\0");
	    }

		strcat(parte_entera,",\0");
		memset(total,'\0',20);
		strcpy(total,parte_entera);
		strcat(total,parte_decimal);
		strcat(total,"\0");

		quita_ceros(total, total_net); // Elimina 000 de 000234,56
		if(ennum < 0 )
		{
			memset(aux5,'\0',20);
			aux5[0] = '-';
			strcat(aux5,total_net);
			memset(total_net,'\0',20);
			strcpy(total_net,aux5);
		}
     
	}
	else // No tiene decimales
	{
		memset(total,'\0',20);
		memset(total_net,'\0',20);
		strcpy(total_net,"0\0");

		if(ennum != 0) quita_ceros(sense_lletra, total_net);

	}

	strcat(total_net,";\0");
	strcat(linea_registro,total_net);

}



void quita_ceros(char total[], char total_net[])
{
	long lon;
	int k, m, pos_ini;

	lon = strlen(total);

	for(k = 0; k <= lon; k++)
	{
		if(total[k] != '0')
		{
			if(total[k] == ',') k--;
			memset(total_net,'\0',20);
			pos_ini = k;
			m = 0;
			for(k = pos_ini; k <= lon; k++)
			{
				total_net[m] = total[k];
				m = m + 1;
			}
		}
	}
}


long extrae_numero(char leido[], char extraido_str[])
{

	int longi,i,k;
	char aux[50];

	longi = strlen(leido);
	memset(aux,'\0',45);
	memset(extraido_str,'\0',45);
	k = 0;

    for( i = 1; i <= longi; i = i + 2)
	{
		aux[k] = leido[i];
		k++;
	}

	strcpy(extraido_str,aux);
	return (long)atol(aux);
}


// Dado un código EBCDIC, busca su equivalente en ASCII
void mi_dictionary(char valor_a_buscar[], char valor_encontrado[])
{

	// Mi Dictionary ------
	char *ptr;
	char clave[4];
	char miDictionary[470] = "4B:.#4C:<#4D:(#4E:+#4F:|#50:&#BB:!#5B:$#5C:*#5D:)#5E:;#6B:,#6C:%#6D:_#6E:>#6F:?#79:`#7A::#69:##7C:@#7E:=#81:a#82:b#83:c#84:d#85:e#86:f#87:g#88:h#89:i#C0:{#91:j#92:k#93:l#94:m#95:n#96:o#97:p#98:q#99:r#D0:}#A2:s#A3:t#A4:u#A5:v#A6:w#A7:x#A8:y#A9:z#4A:[#C1:A#C2:B#C3:C#C4:D#C5:E#C6:F#C7:G#C8:H#C9:I#D1:J#D2:K#D3:L#D4:M#D5:N#D6:O#D7:P#D8:Q#D9:R#E2:S#E3:T#E4:U#E5:V#E6:W#E7:X#E8:Y#E9:Z#F0:0#F1:1#F2:2#F3:3#F4:4#F5:5#F6:6#F7:7#F8:8#F9:9#61:/#9B:§#9A:¦#7B:¥#40: #7D:'#60:-#00:!\0";

	ptr = &miDictionary[0];
	strcpy(valor_encontrado,"NULL\0");

	for(int i=0; i <= 93; i++)
	{

		clave[0] = *ptr;
		clave[1] = *(ptr + 1);
		clave[2] = '\0';

		if(! strcmp(clave, valor_a_buscar))
		{
			valor_encontrado[0] = *(ptr + 3);
			valor_encontrado[1] = '\0';
			i = 700;
		}
		else
		{
			ptr += 5;
		}
		
	}

}


// Carga el contenido del archivo "plantilla" en los arrays
void informa_arrays(char tiene_signo[], char tipo[], int num_dec[], int ocupa_host[], int ocupa_editat[], int pos_ini[])
{

	long i,k,n;
	char linea_leida[90];
	char *p;


	fseek(arxiuPlant,0l,0);

	for(i = 0; i<= lineas_copy - 1; i++) // Nº de variables que hi ha a la copy
	{

	    for(k = 0; k <= 36; k++) linea_leida[k] = '\0';
		fread(linea_leida, 25 * sizeof(char),1,arxiuPlant);
		n = 0;
		p = strtok(linea_leida, "#");
		tiene_signo[i] = p[0];

		do {
			p=strtok('\0', "#");
			if(p)
			{
				++n;
				switch (n) {   
	    			case 1: tipo[i] = p[0]; break;
    				case 2: num_dec[i] = atoi(p); break;
    				case 3: ocupa_host[i] = atoi(p); break;
    				case 4: ocupa_editat[i] = atoi(p); break;
					case 5: pos_ini[i] = atoi(p); break;
					case 6: break;
    				default: puts("Valor no valido para n..."); break;
				}

			} // De if

		} while(p);


		/*printf("%s %c %s", "Signo:", tiene_signo[i],"\n");
		printf("%s %c %s", "Tipo:", tipo[i],"\n");
		printf("%s %d %s", "Num dec:", num_dec[i],"\n");
		printf("%s %d %s", "Ocupa host:", ocupa_host[i],"\n");
        printf("%s %d %s", "Ocupa editat:", ocupa_editat[i],"\n");
		printf("%s %d %s", "Pos ini:", pos_ini[i],"\n");   
		getchar(); */

		fseek(arxiuPlant,0l,1);  

	} // De for

	cerrar_archivo_regs();
	cerrar_archivo_plant();
	
}





//----- Tratamiento archivos -----------------------------------------------------------


// Abre el archivo .BIN
int abrir_archivo_bin(void)
{
	char mensaje[90];
	memset(mensaje,'\0',80);

	sprintf(path_archivo_bin,"%s","C:\\COMP-READER\\");
	strcat(path_archivo_bin , nombre_BIN);

	if((arxiuBin = fopen(path_archivo_bin, "rb")) == NULL)
	{
        strcpy(mensaje, "Error! No he podido abrir el archivo .BIN C:\\COMP-READER\\\0");
		strcat(mensaje,nombre_BIN);
		strcat(mensaje,"\0\0");
	    escribir_aviso(mensaje); // Deja mensaje a Excel 
		printf("%s %s %s", "No he podido abrir el archivo: " , path_archivo_bin, ". Cancelo. . . \n");
		exit(1);
	}
	else
	{
		printf("%s%s %s","Ok, he abierto: " , path_archivo_bin , "\n");
	}

	return 0;
}

// Crea el archivo en donde dejará los registros ( tamaño indicado por "longitut_copy" ) 
// que van a ser tratados.
int crear_archivo_regs(void)
{
	char mensaje[90];
	memset(mensaje,'\0',80);

	char archivoAux[] = "C:\\COMP-READER\\REGS.BIN";

	if(! _access(archivoAux, 00)) remove(archivoAux); //Si existeix, l'esborra

	if((arxiuRegs = fopen("C:\\COMP-READER\\REGS.BIN", "w+")) == NULL)
	{
		strcpy(mensaje, "Error! No he podido crear el archivo 'C:\\COMP-READER\\REGS.BIN.'\0");
		strcat(mensaje,"\0\0");
	    escribir_aviso(mensaje); // Deja mensaje a Excel 
		printf("%s %s %s", "No he podido crear el archivo 'C:\\COMP-READER\\REGS.BIN.' Cancelo. . . \n");
		exit(1);
	}	
	else
	{
		printf("Ok, he creado : C:\\COMP-READER\\REGS.BIN\n\n");
	}

	return(0);

}

// Para que Windows se entere que C ya ha terminado
int crear_archivo_aviso(void) 
{

	char mensaje[90];
	char nomArxiu[] = "C:\\COMP-READER\\XLS_COMMAREA.TXT";

	memset(mensaje,'\0',80);

	if(! _access(nomArxiu, 00)) remove(nomArxiu); //Si existeix, l'esborra

	if((arxiuAvis = fopen("C:\\COMP-READER\\XLS_COMMAREA.TXT", "w+")) == NULL)
	{
		strcpy(mensaje, "Error! No he podido crear el archivo 'C:\\COMP-READER\\REGS.BIN.'\0");
		strcat(mensaje,"\0\0");
	    escribir_aviso(mensaje); // Deja mensaje a Excel 
		printf("%s %s %s", "No he podido crear el archivo 'C:\\COMP-READER\\XLS_COMMAREA.TXT.' Cancelo. . . \n");
		getchar();
		exit(1);
	}	
	else
	{
		printf("Ok, he creado : C:\\COMP-READER\\XLS_COMMAREA.TXT\n");
		fseek(arxiuAvis,0l,0);
		strcpy(mensaje, "Aun no he terminado\0\0\0\0");
	    fwrite(mensaje, 20 * sizeof(char),1,arxiuAvis);
		fflush(arxiuAvis);
		fclose(arxiuAvis);
	}

	return(0);

}



int crear_archivo_salida(void) 
{

	char mensaje[90];
	memset(mensaje,'\0',80);

	char nomArxiu[] = "C:\\COMP-READER\\REGS_XLS.TXT";

	if(! _access(nomArxiu, 00)) remove(nomArxiu); //Si existeix, l'esborra

	if((arxiuXls = fopen("C:\\COMP-READER\\REGS_XLS.TXT", "w+")) == NULL)
	{
		strcpy(mensaje, "Error! No he podido crear el archivo 'C:\\COMP-READER\\REGS_XLS.TXT.'\0");
		strcat(mensaje,"\0\0");
	    escribir_aviso(mensaje); // Deja mensaje a Excel 
		printf("%s %s %s", "No he podido crear el archivo 'C:\\COMP-READER\\REGS_XLS.TXT.' Cancelo. . . \n");
		exit(1);
	}	
	else
	{
		// Ok, archivo creado
		fseek(arxiuAvis,0l,0);
	}

	return(0);

}

int graba_linea_registro(char linea_registro[])
{	
		long longitut;

		strcat(linea_registro, "\n\0");
		longitut = strlen(linea_registro);
	    fwrite(linea_registro, longitut * sizeof(char),1,arxiuXls);
		fflush(arxiuXls);
		fseek(arxiuXls,0l,1);
		return 0;
	
}

int escribir_aviso(char mensaje[])
{
	if((arxiuAvis = fopen("C:\\COMP-READER\\XLS_COMMAREA.TXT", "w+")) == NULL)
	{

		printf("%s %s %s", "\n\nNo he podido abrir el archivo 'C:\\COMP-READER\\XLS_COMMAREA.TXT.' Cancelo. . . \n");
		getchar();
		exit(1);
	}	
	else
	{

		fseek(arxiuAvis,0l,0);
		strcat(mensaje,"\0\0");
		fwrite(mensaje, 80 * sizeof(char),1,arxiuAvis);
		fflush(arxiuAvis);
		fclose(arxiuAvis);
	}

	return(0);

}


// Abre el archivo de plantilla
int abrir_archivo_plantilla(void)
{
	char mensaje[90];
	memset(mensaje,'\0',80);

	printf("%s","\n");
	sprintf(path_archivo_plant,"%s","C:\\COMP-READER\\PLANTILLA.TXT");

	if((arxiuPlant = fopen(path_archivo_plant, "r")) == NULL)
	{
		strcpy(mensaje, "Error! No he podido abrir el archivo 'C:\\COMP-READER\\PLANTILLA.TXT '\0");
		strcat(mensaje,"\0\0");
	    escribir_aviso(mensaje); // Deja mensaje a Excel 
		printf("%s %s %s", "No he podido abrir el archivo: " , path_archivo_bin, ". Cancelo. . . \n");
		exit(1);
	}
	else
	{
		//printf("%s","Ok, \nArchivo plantilla abierto\n");
	}	

	return 0;
}



// Abre el archivo de los registros
int abrir_archivo_regs(void)
{
	char mensaje[90];
	memset(mensaje,'\0',80);

	printf("%s","\n");
	sprintf(path_archivo_regs,"%s","C:\\COMP-READER\\REGS.BIN");

	if((arxiuRegs = fopen(path_archivo_regs, "rb")) == NULL)
	{
		strcpy(mensaje, "Error! No he podido abrir el archivo 'C:\\COMP-READER\\REGS.BIN'\0");
		strcat(mensaje,"\0\0");
	    escribir_aviso(mensaje); // Deja mensaje a Excel 
		printf("%s %s %s", "No he podido abrir el archivo registros: " , path_archivo_regs, ". Cancelo. . . \n");
		exit(1);
	}
	else
	{
		//printf("Ok, he abierto: C:\\COMP-READER\\REGS.BIN\n");
	}

	return 0;
}



void cerrar_archivo_plant(void)
{
	fclose(arxiuPlant); // Tanca arxiu plantilla
	//printf("Ok, he cerrado: C:\\COMP-READER\\PLANTILLA.TXT\n");
}

void cerrar_archivo_regs(void)
{
	fclose(arxiuRegs); // Tanca arxiu registros
	//printf("%s","Ok, he cerrado C:\\COMP-READER\\REGS.BIN\n");
}

void cerrar_archivo_xls(void)
{
	fflush(arxiuXls);
	fclose(arxiuXls); // Tanca arxiu registres Excel
	//printf("ok, he cerrado: C:\\COMP-READER\\REGS_XLS.TXT\n");
}


void cerrar_archivos(void)
{
	fclose(arxiuBin); // Tanca arxiu entrada
	fclose(arxiuRegs); // Tanca arxiu auxiliar
	//puts("\nArchivos cerrados . . .");
}


// Fin tratamiento archivos -------------------------------------------------------------------------



//------ Limpiezas --------------------------------------------------------------------------------


// Inicialitza arrays
void limpiar_arrays(char tiene_signo[], char tipo[], int num_dec[], int ocupa_host[], int ocupa_editat[], int pos_ini[])
{


	memset(tiene_signo,'\0',lineas_copy);
	memset(tipo,'\0',lineas_copy);
	memset(num_dec,'\0',lineas_copy);
	memset(ocupa_host,'\0',lineas_copy);
	memset(ocupa_editat,'\0',lineas_copy);
	memset(pos_ini,'\0',lineas_copy);


}

void limpiar_campos(unsigned char datosLeidos[], char todo[])
{
	
	memset(datosLeidos,'\0',longitut_copy);
	memset(todo,'\0',longitut_copy*2);

}


// ---- Fin limpiezas --------------------------------------------------------------------------



void imprimir_parametros(argc,argv)  int argc; char *argv[];
{
	int i;

	for(i = 0; i < argc; i++)
	{
		printf("%s%d" "Num parametros: " , argc);
		printf("%s%d%s %s %s","Arg-",i,":",argv[i], "\n");
	}

}


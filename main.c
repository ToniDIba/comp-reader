
//Versió 1.0 ( 08.07.2006 )

//-------------------------------------------------------------------------------------
// Comp Reader Fast ! --> Llegeix arxiu de dades HOST baixat per FTP en binary
// i converteix , tot seguint el diseny de la copy associada ( fitxer plantilla ),
// els registres en dades "llegibles" ( ASCII ).
//--------------------------------------------------------------------------------------



#define WIN32_LEAN_AND_MEAN  /* speed up compilations */
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
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
#include <time.h>
#include <process.h>

#include "main.h" // Ojo: Aqui he puesto también los prototipos de las funciones
#include "resource.h"



// OJO: ESTO ES MUY IMPORTANTE **********************************************
//
//
// Para activar el DEBUG:
//     Ve a project/project options / 
//      en la pestaña compiler / combo debug information, pon "Full"
//      en la pestaña linker / combo debug information pon: codeview & COFF. Activa "Verbose"
//
//
//


//El nº de parámetros debe ser 5 por narices. A saber:
// Parametres entrada:  0 - Nom del executable ( no usat )
//                      1 - Nom del .BIN
//                      2 - Num. regs. a omitir
//                      3 - Longitut de la Copy
//                      4 - Longitut del arxiu
//                      5 - Num lineas copy
//                      6 - Condiciones where
	
//    CORTADO.BIN 0 300 300000 30 NOWHERE

// Project | project options | command line arguments ( per fer proves )
//    GHBO048.BIN 0 1000 130000 113 #19:VALENCIA#
//    WCICART.BIN 126600 600 75969600 213 NOWHERE
//    CORTADO.BIN 0 600 1800 213 NOWHERE
//    XEC.BIN 0 1500 465000 264 NOWHERE
//    CORTADO.BIN 0 1500 1500 264 NOWHERE <<-- un solo registro
//    CORTADO.BIN 0 3000 4096 28 NOWHERE

// Para hacer "Displays" usa:
//    TextOut(hdc,10,20,"Iniciado a las: ",16);
//    void mensaje1(HWND hwnd) { MessageBox(hwnd,"Estoy dentro del primero","Texto titulo",MB_OK); }


// I M P O R T A N T E ------------------------------------------------------------------
//
//   Los prototipos de las funciones o métodos o como lo quieras llamar, estan dentro
//   del archivo "main.h".
// --------------------------------------------------------------------------------------



//-------- Paràmetres de entrada suministrados desde la llamada que hace Excel --------

char nombre_BIN[30];     // Nombre del archivo binario ( bajado via FTP ) a tratar.
long regs_a_omitir;      // Número de registros que se van a omitir ( los primeros "n" )
long longitut_copy;      // Lo que mide un registro lógico
long longitut_arxiu_bin; // Ta claro, ¿ no ?
int  lineas_copy;        // Para saber el nº de campos que tiene el registro
char where[80];          // Condiciones de búsqueda ( si las hubiere )

//--------------------------------------------------------------------------------------


//----------- Archivos -----------------------------------------------------------------

FILE *arxiuBin;    // Arxiu .BIN
FILE *arxiuPlant;  // Arxiu de plantilla ( Si el campo tiene signo, si es COMP-3, Alfa . . .
FILE *arxiuRegs;   // nº regs. del arxiu entrada .BIN
FILE *arxiuAvis;   // Para intercambio de avisos entre Excel y este módulo
FILE *arxiuXls;    // Registro ( campos delimitados por ";" ) que luego será importado desde Excel

/** Global variables ********************************************************/

static HANDLE ghInstance;
       WNDCLASS wc;
       HDC hdc;
static HWND hwnd;
	   MSG Msg;


// Diálogo "Acerca de"
BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
		case WM_INITDIALOG:

		return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDOK:
					EndDialog(hwnd, IDOK);
				break;
				case IDCANCEL:
					EndDialog(hwnd, IDCANCEL);
				break;
			}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}


//************************************************************************************************
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
//************************************************************************************************
{

	int retorno = 0 ;
	float prueba = 0.0f;


	retorno = registrarClase(hInstance);
	if(! retorno) mensajeError(0); // Error al registrar ventana
	retorno = crearVentana(hInstance);
	if(! retorno) mensajeError(1); // Error al crear la ventana
	

	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE );

	capturaHora();
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);


	// Mi código ------------------------------------------------------------------------------
	retorno = parametros_entrada(lpCmdLine, hwnd);      // Comprueba / asigna parámetros
	if(! retorno) // Retorno = 0 indica que han llegado todos los parámetros
	{
		
		EnableWindow(hwnd,FALSE);
        prueba = 2.34 * 5.23;
		crear_archivo_aviso();       // Para que Excel sepa que C ha terminado
		abrir_archivo_bin();         // Abre archivo .BIN
		crear_archivo_regs();        // Crea archivo auxiliar donde dejar registros del .BIN
		llenar_archivo_regs();       // Crea n registros a partir del archivo de entrada
		tratar_archivo_regs();       // Formatea registro segun archivo plantilla
		cerrar_archivos();
		procesos_finales(); 
		exit(0);
	}
    //-----------------------------------------------------------------------------------------

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}


	return Msg.wParam;
}



LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{

	HDC         hdc;
    PAINTSTRUCT ps;
    RECT        rc;
	int longi=0;


	
	switch(Message)
	{

       case WM_PAINT:
    	BeginPaint(hwnd, &ps);
    	GetClientRect(hwnd, &rc);
		hdc = GetDC(hwnd);
		longi = strlen(miTexto);
		TextOut(hdc,10,20,miTexto, longi);
    	EndPaint(hwnd, &ps);
		return 0;

	   case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_HELP_ABOUT:
				{
					DialogBox(GetModuleHandle(NULL), 
					MAKEINTRESOURCE(IDD_ABOUT), hwnd, AboutDlgProc);
				}
				break;
			}
		break;
		case WM_KEYDOWN:
            // Ver KeyPressed()
        break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}



int parametros_entrada(LPSTR lpCmdLine, HWND hwnd)
{
	int cuantos_params = 0;

	cuantos_params = asignar_parametros(lpCmdLine);

	//El nº de parámetros debe ser 5 por narices. A saber:
	// Parametres entrada:  0 - Nom del executable ( no usat )
    //                      1 - Nom del .BIN
    //                      2 - Num. regs. a omitir
    //                      3 - Longitut de la Copy
	//                      4 - Longitut del arxiu
	//                      5 - Num lineas copy
	//                      6 - Condiciones where


	if(cuantos_params != 5)
	{
		 MessageBox(hwnd, "Faltan parámetros de entrada. " , "Error",MB_OK | MB_ICONERROR);
		 return -1;
	}
	
	return 0;

}




// Comprueba que el nº de parámetros sea el correcto ( 6 ), y asigna dichos parámetros
// a variables del programa.
//***********************************************
int asignar_parametros(LPSTR lpCmdLine)
//***********************************************
{
	
	char *p;
	int cuantos = 0;
	char puntuado[50];
	char aux[225];
	char buf[250];


    if(strlen(lpCmdLine)== 0) return 0;

	p = strtok(lpCmdLine, " ");
	sprintf(nombre_BIN , p);

	do {
		p=strtok('\0' , " "); //Tokenizer: Devuelve "tokens" separados (en este caso) por espacios o final de cadena

		if(p)
		{
            ++cuantos;
            switch (cuantos)
			 {   
   				case 1: regs_a_omitir = atol(p); break;
    			case 2: longitut_copy = atol(p); break;
    			case 3: longitut_arxiu_bin = atol(p); break;
				case 4: lineas_copy = atol(p); break;
    			case 5: sprintf(where,p); break;
    			default: puts("not a clue what cuantos is..."); break;
			}
		}

	} while(p);


	 memset(aux,'\0',220);
	 strcpy(aux,"Nombre .BIN    : ");
	 strcat(aux, nombre_BIN );
	 strcat(aux, "\n");

	 strcat(aux, "Regs. omitidos : ");
	 memset(puntuado,'\0',47);
	 puntuar_numero(_ultoa(regs_a_omitir,buf,10), puntuado);
	 strcat(aux, puntuado );
	 strcat(aux, "\n");

	 strcat(aux, "Longitud copy  : ");
	 memset(puntuado,'\0',47);
	 puntuar_numero(_ultoa(longitut_copy,buf,10), puntuado);
	 strcat(aux, puntuado);
	 strcat(aux, "\n");

	 strcat(aux, "Longitud .BIN   : ");
	 memset(puntuado,'\0',47);
	 puntuar_numero(_ultoa(longitut_arxiu_bin,buf,10), puntuado);
	 strcat(aux, puntuado);
	 strcat(aux, "\n");

	 strcat(aux, "Lineas copy      : ");
	 memset(puntuado,'\0',47);
	 puntuar_numero( _ultoa(lineas_copy,buf,10), puntuado);
	 strcat(aux, puntuado);
	 strcat(aux, "\n");

	 strcat(aux, "Filtro Where     : ");
	 strcat(aux, where );

	 MessageBox(NULL, aux  ,"  CompReader.c       Parámetros de entrada ",MB_OK | MB_ICONINFORMATION);
	
	 return cuantos;
}





//********************************
int crear_archivo_aviso(void)   // Para que Windows se entere que C ya ha terminado
//********************************
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
		MessageBox(NULL, "Error al crear \"C:\\COMP-READER\\XLS_COMMAREA.TXT.\" " , "Error",MB_OK | MB_ICONERROR);
		exit(1);
	}	
	else
	{
		fseek(arxiuAvis,0l,0);
		strcpy(mensaje, "Aun no he terminado\0\0\0\0");
	    fwrite(mensaje, 20 * sizeof(char),1,arxiuAvis);
		fflush(arxiuAvis);
		fclose(arxiuAvis);
	}

	return(0);

}

//*********************************
int escribir_aviso(char mensaje[]) // Escribe un mensaje que será leido desde Excel
//*********************************
{
	if((arxiuAvis = fopen("C:\\COMP-READER\\XLS_COMMAREA.TXT", "w+")) == NULL)
	{
		MessageBox(NULL, "No he podido abrir el archivo \"C:\\COMP-READER\\XLS_COMMAREA.TXT.\"" , "Error",MB_OK | MB_ICONERROR);
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


//*****************************
int abrir_archivo_bin(void)    // Abre el archivo .BIN
//*****************************
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
		MessageBox(NULL, mensaje , "Error",MB_OK | MB_ICONERROR);
		exit(1);
	}

	return 0;
}


//Captura la hora del sistema para mostrarla al principio
void capturaHora(void)
{
	char hora[5];
	char minu[5];
	char segu[5];

	char sbuf[12];


	GetLocalTime(&temps);
	memset(miTexto,'\0',75);
	memset(hora,'\0',4);
	memset(minu,'\0',4);
	memset(segu,'\0',4);
	if(temps.wHour <= 9) strcat(hora,"0\0");
	if(temps.wMinute <= 9) strcat(minu,"0\0");
	if(temps.wSecond <= 9) strcat(segu,"0\0");
    strcat(hora, _itoa(temps.wHour, sbuf, 10));
	strcat(hora,"\0");
	strcat(minu, _itoa(temps.wMinute, sbuf, 10));
	strcat(minu,"\0");
	strcat(segu, _itoa(temps.wSecond, sbuf, 10));
	strcat(segu,"\0");
	strcpy(miTexto,"Iniciado a las ");
	strcat(miTexto,"\0");
	strcat(miTexto,hora);
	strcat(miTexto,":\0");
	strcat(miTexto,minu);
	strcat(miTexto,":\0");
	strcat(miTexto,segu);

}



//********************************
int crear_archivo_regs(void)     // Crea el archivo en donde dejará los registros ( tamaño indicado por "longitut_copy" ) // que van a ser tratados.
//********************************
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
		MessageBox(NULL, mensaje , "Error",MB_OK | MB_ICONERROR);
		exit(1);
	}	

	return(0);

}

//******************************
int crear_archivo_salida(void) 
//******************************
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
		MessageBox(NULL, mensaje , "Error",MB_OK | MB_ICONERROR);
		exit(1);
	}	
	else
	{
		// Ok, archivo creado
		fseek(arxiuAvis,0l,0);
	}

	return(0);

}



//***********************************
void llenar_archivo_regs(void)
//***********************************
{
	long numRegs;    // Según la copy, el nº de registros que existen en el .BIN
	long desplBin;   // Offset del .BIN
	long desplRegs;  // Offset del archivo auxiliar con los registros a tratar
	long longitut_read_1 = 0;
	long longitut_read_2 = 0;
	char en_hexa[] = "  ";
	char mensaje[90];
	char en_string[40];
	int k, i;
	long cuantos = 0;

	unsigned char *datosLeidos; // Crea un array dinàmic
	datosLeidos = (unsigned char *)malloc(longitut_copy);
	
	int lon = longitut_copy * 2;
	char str1[lon];
	char todo[lon];
	char bufs[50];
	char aux2[120];


	// -------------------------------------------------------------------------------------------


	numRegs = longitut_arxiu_bin / longitut_copy;
	memset(bufs,'\0',40);
    memset(aux2,'\0',100);

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

    longitut_read_1 = ( longitut_copy * sizeof(unsigned char) );
	longitut_read_2 = ( longitut_copy * sizeof(char)*2 );
	(void) time(&t1);

	for(i = (regs_a_omitir + 1) ; i <= numRegs; i++)
	{	

		limpiar_campos(datosLeidos, todo); 
		fseek(arxiuBin,desplBin,0);
		fread(datosLeidos, longitut_read_1 , 1 , arxiuBin);

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
		fwrite(&todo, longitut_read_2 ,1,arxiuRegs);
		fflush(arxiuRegs);
		(void) time(&t2);
		cuantos++;
		
	   
		if( (int) t2-t1 >= 1 ) // Si ha transcurrido 1 segundo o más, enviar mensaje a Excel
		{
		   (void) time(&t1);
		   memset(en_string,'\0',35);
		   _ultoa(cuantos, en_string,10);
		   memset(mensaje,'\0',80);
		   strcat(mensaje, "Tratados paso 1: \0");
		   strcat(mensaje, en_string);
		   strcat(mensaje,"\0\0");
		   escribir_aviso(mensaje);
		}

		//Si pulsan ESC, cancelo
		if (KeyPressed(VK_ESCAPE) == 1)
		{ 
			memset(mensaje,'\0',80);
			strcat(mensaje, "Han pulsado ESC.\0");
			escribir_aviso(mensaje);
			_fcloseall();
			exit(0);
		}

		todo[0] = '\0';
		desplBin  += longitut_copy;
		desplRegs += (longitut_copy * 2);
		
	}

}


//Llegeix registre a registe arxiu registres i formateja les dades segons dicti arxiu plantilla
int tratar_archivo_regs(void)
{
	
	long regs_a_tractar;
	long i,k,ik,longi,longi_edit;
	long cuantos = 0;
	int num_decs;

	char signo[4];
	char mensaje[90];
	char en_string[40];
	char patron[10];
	char carac2;
	char valor_where[1000];
	char valor_registro[1000];
    char cumplidoras[2000];
	char bufs[60];
	char aux2[225];
	

	// Donde guardo contenido archivo "plantilla"
	char tiene_signo[lineas_copy];
	char tipo[lineas_copy];
	int num_dec[lineas_copy];
	int ocupa_host[lineas_copy];
	int ocupa_editat[lineas_copy];
	int pos_ini[lineas_copy];
	char linea_registro[longitut_copy + lineas_copy]; // + ';' per l'excel





	regs_a_tractar = (long) ( longitut_arxiu_bin / longitut_copy );
	if(regs_a_omitir > 0) regs_a_tractar -= regs_a_omitir;
	
	abrir_archivo_plantilla();   // Abre archivo con la definición del registro
	abrir_archivo_regs();        // Abre archivo con los registros a tratar
	limpiar_arrays(tiene_signo, tipo, num_dec, ocupa_host, ocupa_editat, pos_ini); // Inicialitza arrays
	informa_arrays(tiene_signo, tipo, num_dec, ocupa_host, ocupa_editat, pos_ini); // Plantilla >> arrays


	crear_archivo_salida();
	abrir_archivo_regs();
	fseek(arxiuRegs,0l,0);


  	memset(aux2,'\0',220);
	strcpy(aux2,"Registros a tratar : ");
	strcat(aux2, _ultoa(regs_a_tractar,bufs,10) );
    memset(mensaje,'\0',80);
    strcat(mensaje, "Entrado en paso 2: \0");
    //strcat(mensaje, en_string);
    strcat(mensaje,"\0\0");
	escribir_aviso(mensaje);

	printf("%s", "Hola toni");

	(void) time(&t1);

	for(ik = 0; ik <= regs_a_tractar - 1; ik++)  // Trato registro
	{

		cuantos++;

	    (void) time(&t2);

		if( (int) t2-t1 >= 1 ) // Si ha transcurrido 1 segundo o más, enviar mensaje a Excel
		{
		   (void) time(&t1);
		   memset(en_string,'\0',35);
		   _ultoa(cuantos, en_string,10);
		   memset(mensaje,'\0',80);
		   strcat(mensaje, "Tratados paso 2 de segs: \0");
		   strcat(mensaje, en_string);
		   strcat(mensaje,"\0\0");
		   escribir_aviso(mensaje);
		}


		// Si pulsan ESC, cancelo
		if (KeyPressed(VK_ESCAPE) == 1)
		{ 
			memset(mensaje,'\0',80);
			strcat(mensaje, "Han pulsado ESC.\0");
			escribir_aviso(mensaje);
			_fcloseall();
			exit(0);
		} 

		memset(linea_registro,'\0',(longitut_copy + lineas_copy));

		for(k = 0; k <= lineas_copy - 1; k++) // Dentro de cada registro, variable a variable
		{
          

			printf("%s %d %s", "Linea copy  :", k,"\n");
			printf("%s %c %s", "Signo  :", tiene_signo[k],"\n");
			printf("%s %c %s", "Tipo   :", tipo[k],"\n");
			printf("%s %d %s", "Num dec:", num_dec[k],"\n");
			printf("%s %d %s", "Ocupa Host  :", ocupa_host[k],"\n");
			printf("%s %d %s", "Ocupa editat:", ocupa_editat[k],"\n");
			printf("%s %d %s", "Pos ini:", pos_ini[k],"\n");


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
    			default: 
                          memset(mensaje,'\0',80);
		                  strcat(mensaje, "Tipo desconocido. \0");
		                  strcat(mensaje, en_string);
		                  strcat(mensaje,"\0\0");
		                  escribir_aviso(mensaje);
						  exit(0);

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

	TextOut(hdc,10,40,"Terminado a las: ",16);
	MessageBox(NULL, "Proceso finalizado" , " CompReader.c",MB_OK | MB_ICONINFORMATION);
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
	/*char leido[1500]; */
	char leido[8000];
	char valor_a_buscar[5];
	char valor_encontrado[6];
	int lonx, i;
	//char aux[1000];
	char aux[8000];
	char nulo[40];

	//memset(leido,'\0',1490);
	memset(leido,'\0',7990);
	fread(&leido, ( longi * sizeof(char)) * 2 ,1,arxiuRegs);
	fseek(arxiuRegs,0l,1);

	lonx = strlen(leido);
	//memset(aux,'\0',998);
	memset(aux,'\0',7998);
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


// Abre el archivo de plantilla
int abrir_archivo_plantilla(void)
{
	char mensaje[90];
	memset(mensaje,'\0',80);

	sprintf(path_archivo_plant,"%s","C:\\COMP-READER\\PLANTILLA.TXT");

	if((arxiuPlant = fopen(path_archivo_plant, "r")) == NULL)
	{
		strcpy(mensaje, "Error! No he podido abrir el archivo 'C:\\COMP-READER\\PLANTILLA.TXT '\0");
		strcat(mensaje,"\0\0");
	    escribir_aviso(mensaje); // Deja mensaje a Excel 
	    MessageBox(NULL, mensaje , "Error",MB_OK);
		exit(1);
	}

	return 0;
}



// Abre el archivo de los registros
int abrir_archivo_regs(void)
{
	char mensaje[90];
	memset(mensaje,'\0',80);

	sprintf(path_archivo_regs,"%s","C:\\COMP-READER\\REGS.BIN");

	if((arxiuRegs = fopen(path_archivo_regs, "rb")) == NULL)
	{
		strcpy(mensaje, "Error! No he podido abrir el archivo 'C:\\COMP-READER\\REGS.BIN'\0");
		strcat(mensaje,"\0\0");
	    escribir_aviso(mensaje); // Deja mensaje a Excel 
    	MessageBox(NULL, mensaje , "Error",MB_OK);
		exit(1);
	}

	return 0;
}


// Carga el contenido del archivo "plantilla" en los arrays
void informa_arrays(char tiene_signo[], char tipo[], int num_dec[], int ocupa_host[], int ocupa_editat[], int pos_ini[])
{

	long i,k,n;
	char linea_leida[90];
	char *p;


	fseek(arxiuPlant,0l,0);

	for(i = 0; i <= lineas_copy - 1; i++) // Nº de variables que hi ha a la copy
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
	//strcpy(valor_encontrado,"NULL\0");
	strcpy(valor_encontrado,"?\0");

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

void procesos_finales(void)
{
	char mensaje[90];
	char en_string[40];
	long regs_a_tractar;

	regs_a_tractar = (long) ( longitut_arxiu_bin / longitut_copy );
	if(regs_a_omitir > 0 ) regs_a_tractar = (long) regs_a_tractar - regs_a_omitir;
	memset(en_string,'\0',35);
	memset(mensaje,'\0',80);
	if(! strcmp(where, "NOWHERE")) // No han puesto where ( quieren todas las columnas )
	{
		_ultoa(regs_a_tractar, en_string,10);
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


int registrarVentana(void)
{

   // Register the main window class.
    wc.lpszClassName = _T("CompReaderClass");
    wc.lpfnWndProc = WndProc;
    wc.style = CS_OWNDC|CS_VREDRAW|CS_HREDRAW;
    wc.hInstance = ghInstance;
    wc.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MNU_MAIN);
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    //
    if (!RegisterClass(&wc)) return 1; else return 0;

}


void cerrar_archivo_plant(void) { fclose(arxiuPlant); } // Tanca arxiu plantilla 
void cerrar_archivo_regs(void) { fclose(arxiuRegs); }   // Tanca arxiu registros 
	
void cerrar_archivo_xls(void)
{
	fflush(arxiuXls);
	fclose(arxiuXls); // Tanca arxiu registres Excel
}

void cerrar_archivos(void)
{
	fclose(arxiuBin); // Tanca arxiu entrada
	fclose(arxiuRegs); // Tanca arxiu auxiliar
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


int registrarClase(HINSTANCE hInstance)
{

	WNDCLASSEX wc;

	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = 0;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MYMENU);
	wc.lpszClassName = g_szClassName;
	//wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); // Sin icono
	wc.hIconSm  = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON), IMAGE_ICON, 16, 16, 0);


	if(!RegisterClassEx(&wc)) return 0;

	return 1;

}


// Crea la ventana
int crearVentana(HINSTANCE hInstance)
{

	hwnd = CreateWindowEx(
		   WS_EX_CLIENTEDGE,
		   g_szClassName,
		   " CompReader Fast!   ",
		   WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
	       CW_USEDEFAULT, CW_USEDEFAULT,250,110,
	       NULL, NULL, hInstance, NULL);

	if(hwnd == NULL) return 0;
	return 1;
}

//Error al registrar la ventana
void mensajeError(int numErr)
{
	
	if(numErr == 0)
	{
		MessageBox(NULL, "Error al registrar la ventana!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		exit(0);
	}
	else
	{
		MessageBox(NULL, "No he podido crear la ventana !", "Error!", MB_ICONEXCLAMATION | MB_OK);
		exit(0);
	}

}


// Comprueba si se ha pulsado ESC para abortar
int KeyPressed(int tecla)
{

	if( GetAsyncKeyState(VK_ESCAPE))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// Puntua millares. Pasa de 123456 a 123.456
void puntuar_numero(char numero[], char puntuado[])
{
	char szBuf[20];

	NUMBERFMT numFormat;
	numFormat.Grouping=3;
	numFormat.LeadingZero=1;
	numFormat.NumDigits=0;
	numFormat.NegativeOrder=0;
	numFormat.lpDecimalSep = ",";
	numFormat.lpThousandSep = ".";
	GetNumberFormat(LOCALE_SYSTEM_DEFAULT, 0, numero, &numFormat, szBuf, sizeof(szBuf) );
	strcpy(puntuado, szBuf);

}


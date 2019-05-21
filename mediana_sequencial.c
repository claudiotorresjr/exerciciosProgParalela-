#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct imagem
{
	char tipo[3];
	int col;
	int lin;
	int corMax;
	int **matriz;
}img;

void leitura_p5( img * pgm, FILE *arquivo);
void leitura_p2(img *pgm, FILE *arquivo);
void leitura(img *pgm);
void impressao(img *pgm, int *saida);
void liberamem(img *pgm);
void mediana(img *pgm, int tam);


void leitura_p5(img * pgm, FILE *arquivo)
{
	unsigned char **matrizaux;
	int i, j;

	matrizaux = malloc(sizeof(unsigned char*)*pgm->lin);
	for( i = 0; i < pgm->lin; ++i)
	{
		matrizaux[i] = malloc(sizeof(unsigned char)*pgm->col);
	}

	for(i = 0 ; i < pgm->lin; ++i)
	{
		fread(matrizaux[i], sizeof(unsigned char), pgm->col, arquivo);
	}

	for(i = 0; i < pgm->lin; ++i)
	{
		for(j = 0; j < pgm->col; j++)
		{
			pgm->matriz[i][j] = matrizaux[i][j];
		}
	}
	
	for(i = 0 ; i < pgm->lin; ++i)
	{
			free(matrizaux[i]);
	}
}

void leitura_p2(img *pgm, FILE *arquivo)
{
	int i, j;

	for(i = 0 ; i < pgm->lin; ++i)
	{
		for ( j = 0 ; j < pgm->col ; ++j)
		{
			 fscanf(arquivo ,"%d ", &pgm->matriz[i][j]);
		}
	}
}

void leitura(img *pgm)
{
	char buff[3][30];
	char tmp[100];
	char linha[200];

	int i = 0;

	FILE *arquivo = fopen("ballons.pgm", "r");

	if(!arquivo) 
		fprintf(stderr, "Erro ao abrir o arquivo!\n");
	else
	{
		while(i < 4)
		{
			fscanf( arquivo, "%s", tmp);
		    if (tmp[0] == '#')
		    {
				fgets(linha, 199, arquivo);
				fscanf(arquivo, "%s", tmp);
		    }
		    strcpy(buff[i] , tmp);
			i++;
		}
		
		//transferindo da "buff" para o registro
		strcpy(pgm->tipo, buff[0]);			
		pgm->col = atoi(buff[1]);				//recebe tam das colunas
		pgm->lin = atoi(buff[2]);				// recebe tam das linhas
		pgm->corMax = atoi(buff[3]);			//recebe valor do branco
		
		//alocar e ler a matriz
		pgm->matriz =  malloc(sizeof(int*)*pgm->lin);
		for(i = 0; i < pgm -> lin; ++i)
			pgm->matriz[i] = (int*)malloc(sizeof(int)*pgm->col);
		
		if(strcmp (pgm->tipo, "P5\0")){
			leitura_p2(pgm, arquivo);
		} else {	
			leitura_p5(pgm, arquivo);
		}
	}
	fclose(arquivo);
}


void impressao(img *pgm, int *saida)
{
	int col;
	int lin;
	int corMax;
	int i,j;

	col = pgm->col;
	lin = pgm->lin;
	corMax = pgm->corMax;
	
	FILE *arquivo = fopen("paralelo.pgm", "w");
	
	fprintf(arquivo , "P2\n");
	fprintf(arquivo, "%d ", col);
	fprintf(arquivo, "%d\n", lin);
	fprintf(arquivo, "%d\n", corMax);
	
	for(i = 0; i < lin; ++i)
	{
		for(j = 0; j < col ; ++j)
		{
			fprintf(arquivo, "%d ", saida[i*lin + j]);
		}
		fprintf(arquivo, "\n");
	}
	fclose(arquivo);
}


void liberamem(img *pgm)
{
	int i;
	for(i = 0 ; i < pgm->lin; ++i)
	{
		free(pgm->matriz[i]);	
	}	
	free(pgm->matriz);
}


void ordena(int *v, int tam)
{
	int x, chave, y;
	for (x = 1; x < tam; ++x)
	{
		chave = v[x];
		y = x - 1;
		while (y >= 0 && v[y] > chave)
		{
			v[y + 1] = v[y];
			y--;
		}
		v[y + 1] = chave;
	}
}

void mediana(img *pgm, int tam)
{

	int metade, *saida;
	int m, i, j, k, l;
	int x, y, chave;
	int tamMask;
	
	int mask[tam*tam];

	saida = (int*)malloc(pgm->lin*pgm->col*sizeof(int));
	
	memset(saida, 0, pgm->lin*pgm->col*sizeof(int));

	metade = (int)(tam)/2;

	tamMask = tam*tam;

	for(i = 0; i < (pgm->lin - tam + 1) ; ++i)
	{
		for(j = 0; j < (pgm->col - tam + 1); ++j)
		{		
			m = 0;	
			for(k = i; k < i + tam; ++k)
			{
				for(l = j; l < j + tam; ++l)
				{			
					mask[m] = pgm->matriz[k][l];
					m++;
				}	
			}
			ordena(mask, tamMask);
			saida[(i+metade)*pgm->lin + (j+metade)] = mask[tamMask/2];
		}
	}

	impressao(pgm, saida);
}

int main(int argc, char const *argv[])
{
	img pgm;
	int intMed;
	double start, end;

	printf("Coloque o tamanho do filtro (numero impar)\n");
	scanf("%d", &intMed);

	if(intMed%2 != 0)
	{
		leitura(&pgm);
		mediana(&pgm, intMed);		
		liberamem(&pgm);	
	}
	else
	{
		printf("tava escrito impar. pra q colocar par?? :#\n");
	}
	
	return 0;

}
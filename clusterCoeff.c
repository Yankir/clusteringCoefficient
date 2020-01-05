#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//tamanho da string dos vertices
#define stringSize 1025

//numero de vertices no grafo
static int vertexCount = 0;

//grafo na forma de matriz de adjacencia
typedef struct grafo
{
	char *graphName;

	//matriz de adjacencia
	int **matrix;
}
*grafo;

//traducao dos vertices para a matriz em si (string --> indice do vertice na matriz)
typedef struct vertice
{
	char *vertexName;
	int vertexIndex;
	struct vertice *nextVertex;
}
*vertice;

static vertice v;

//lista para as arestas
typedef struct edge
{
	int vertex1 , vertex2;
	struct edge *nextEdge;
}
*edge;

static edge e;

//prototipos
int vertexListed(char *vertexName);
void createVertex(char *vertexName);
int findVertex(char *vertexName);
void createEdge(int vertex1 , int vertex2);
grafo setGraph(grafo g);
grafo le_grafo(FILE *input);
double coeficiente_agrupamento_grafo(grafo g);
grafo printMatrix(FILE *output, grafo g);
char *vertexName(int vertexIndex);
grafo escreve_grafo(FILE *output, grafo g);
int destroi_grafo(grafo g);

//retorna se o vertice esta na lista de vertices
int vertexListed(char *vertexName)
{
	vertice aux = v->nextVertex;

	while(aux != NULL)
	{
		//compara se a lista ja tem esse vertice
		if(strcmp(aux->vertexName , vertexName) == 0)
			return 1;

		aux = aux->nextVertex;
	}

	//nao esta na lista
	return 0;
}

//aloca espaco para um novo vertice e liga com a lista de vertices
void createVertex(char *vertexName)
{
	//checa se vertice ja esta na lista
	if(vertexListed(vertexName))
		return;

	//pointer auxiliar para ligar o novo vertice
	vertice aux = malloc(sizeof(vertice));
	aux->vertexName = malloc((strlen(vertexName) + 2) * sizeof(char));

	//adiciona informacoes do vertice
	strcpy(aux->vertexName , vertexName);
	aux->vertexIndex = vertexCount;

	//arruma os pointers
	aux->nextVertex = v->nextVertex;
	v->nextVertex = aux;

	//+1 vertice para a lista
	vertexCount++;
}

//procura a traducao do vertice para a matriz
int findVertex(char *vertexName)
{
	vertice aux = v->nextVertex;

	while(aux != NULL)
	{
		//retorna o indice desse vertice
		if(strcmp(aux->vertexName , vertexName) == 0)
			return aux->vertexIndex;

		aux = aux->nextVertex;
	}

	//erro
	return -1;
}

//aloca espaco para uma nova aresta e liga com a lista de arestas
void createEdge(int vertex1 , int vertex2)
{
	//pointer auxiliar para ligar a nova aresta
	edge aux = malloc(sizeof(edge));

	aux->vertex1 = vertex1;
	aux->vertex2 = vertex2;

	aux->nextEdge = e->nextEdge;
	e->nextEdge = aux;
}

//poe 0 em todas as posicoes da matriz de adjacencia
grafo setGraph(grafo g)
{
	for(int i = 0 ; i < vertexCount ; i++)
	{
		for(int j = 0 ; j < vertexCount ; j++)
			g->matrix[i][j] = 0;
	}

	return g;
}

grafo le_grafo(FILE *input)
{
	//caso de erro
	if(input == NULL)
		return NULL;

	//line vai receber linha inteira para depois ser dividida
	char line[stringSize];
	char v1[stringSize] , v2[stringSize];

	//cabeca para a lista de vertices/arestas
	v = malloc(sizeof(vertice));
	e = malloc(sizeof(edge));

	v->nextVertex = NULL;
	e->nextEdge = NULL;

	while(!feof(input))
	{
		//pega a linha
		fgets(line , 2*stringSize + 1 , input);
		//remove \n da string
		strtok(line , "\n");

		//procura o espaco e da a primeira string para v1
		char *aux;
		aux = strtok(line , " \t");
		if(aux != NULL)
			strcpy(v1 , aux);
		else
			strcpy(v1 , " ");

		//procura o fim e da a outra string para v2
		while(aux != NULL)
		{
			strcpy(v2 , aux);
			aux = strtok(NULL , " \t\0");
		}

		//checa se nao foi uma linha vazia ou vertice isolado
		if(!((strcmp(v1 , " ") == 0) || (strcmp(v2 , " ") == 0) || (strcmp(v1 , "\n") == 0) || (strcmp(v2 , "\n") == 0) || (strcmp(v1 , v2) == 0)))
		{
			//cria os 2 vertices(se ja nao foram criados)
			createVertex(v1);
			createVertex(v2);

			//poe na lista de arestas
			createEdge(findVertex(v1) , findVertex(v2));
		}
	}

	fclose(input);

	//aloca espaco para grafo g
	grafo g = malloc(sizeof(grafo));

	//aloca espaco para as linhas e colunas da matriz
	g->matrix = malloc(vertexCount * sizeof(int *));
	for(int i = 0 ; i < vertexCount ; i++)
		g->matrix[i] = malloc(vertexCount * sizeof(int));

	//zera a matriz
	g = setGraph(g);

	//percorrer a lista de arestas e por na matriz
	edge aux = e->nextEdge;
	while(aux != NULL)
	{
		//poe 1 nas duas posicoes que os vertices se ligam
		g->matrix[aux->vertex1][aux->vertex2] = 1;
		g->matrix[aux->vertex2][aux->vertex1] = 1;

		aux = aux->nextEdge;
	}

	//retorna grafo com a matriz pronta
	return g;
}

double coeficiente_agrupamento_grafo(grafo g)
{
	double open = 0 , closed = 0;

	//percorre todas as linhas olhando os vizinhos
	for (int i = 0; i < vertexCount; ++i)
	{
		for(int j = 0 ; j < vertexCount ; j++)
		{
			//achou o primeiro amigo de i
			if(g->matrix[i][j] == 1)
			{
				//loop para achar outro amigo de i e comparar com j
				for(int k = j+1 ; k < vertexCount ; k++)
				{
					//achou segundo amigo de i
					if(g->matrix[i][k] == 1)
					{
						//ver se os 2 amigos sao amigos entre si --> triade fechada
						if(g->matrix[j][k] == 1)
							closed++;
						//triade aberta
						else
							open++;
					}
				}
			}
		}
	}

	//caso nao houver triades retornar 0
	if(open + closed == 0)
		return 0;

	return closed / (open + closed);
}

//funcao para printar a matriz de adjacencia do grafo
grafo printMatrix(FILE *output, grafo g)
{
	//caso de erro
	if(g == NULL || g->matrix == NULL || g->matrix[0] == NULL || output == NULL)
		return NULL;

	//printa toda a matriz em output
	for(int i = 0 ; i < vertexCount ; i++)
	{
		for(int j = 0 ; j < vertexCount - 1 ; j++)
			fprintf(output , "%d " , g->matrix[i][j]);
		fprintf(output , "%d" , g->matrix[i][vertexCount - 1]);
		if(i != vertexCount - 1)
			fputs("\n" , output);
	}

	fclose(output);

	//retorna grafo lido
	return g;
}

//retorna a string do vertice procurado
char *vertexName(int vertexIndex)
{
	vertice aux = v->nextVertex;
	while(aux != NULL)
	{
		//achou vertice
		if(aux->vertexIndex == vertexIndex)
			return aux->vertexName;

		aux = aux->nextVertex;
	}

	//a busca nunca vai sair do while porque sempre vai existir o vertice sendo procurado
	return NULL;
}

//printa a lista de arestas do grafo
grafo escreve_grafo(FILE *output, grafo g)
{
	//caso de erro
	if(g == NULL || output == NULL)
		return NULL;

	edge aux = e->nextEdge;
	while(aux != NULL)
	{
		//traduzir posicao da matriz para string e printar
		fprintf(output , "%s %s" , vertexName(aux->vertex1) , vertexName(aux->vertex2));

		if(aux->nextEdge != NULL)
			fputs("\n" , output);

		aux = aux->nextEdge;
	}

	fclose(output);

	//retorna grafo lido
	return g;
}

//remove matriz de adjacencia e estruturas intermediarias (lista de vertices/arestas)
int destroi_grafo(grafo g)
{
	//caso de erro
	if(g == NULL || g->matrix == NULL || g->matrix[0] == NULL)
		return 0;

	//libera todas as linhas
	for(int i = 0 ; i < vertexCount ; i++)
	{
		free(g->matrix[i]);
		g->matrix[i] = NULL;
	}

	//libera a matriz em si
	free(g->matrix);
	g->matrix = NULL;

	//liberando lista de vertices
	while(v->nextVertex != NULL)
	{
		vertice prevAux = v , aux = v->nextVertex;

		//percorre a lista ate o ultimo e da free
		while(aux->nextVertex != NULL)
		{
			prevAux = aux;
			aux = aux->nextVertex;
		}
		free(aux);
		prevAux->nextVertex = NULL;
	}
	free(v);
	v = NULL;

	//liberando lista de arestas
	while(e->nextEdge != NULL)
	{
		edge prevAux = e , aux = e->nextEdge;

		//percorre a lista ate o ultimo e da free
		while(aux->nextEdge != NULL)
		{
			prevAux = aux;
			aux = aux->nextEdge;
		}
		free(aux);
		prevAux->nextEdge = NULL;
	}
	free(e);
	e = NULL;

	//retornando normalmente sem erros
	return 1;
}

int main(void)
{
	grafo g = le_grafo(stdin);

	if ( !g )
		return 1;

	printf("O coeficiente de agrupamento do grafo é %f\n", coeficiente_agrupamento_grafo(g));

	return !destroi_grafo(g);
}

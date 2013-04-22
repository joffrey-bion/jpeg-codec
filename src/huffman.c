#include "../include/huffman.h"

/* 
 * 'tab' est le tableau des nombres de codes par longueur
 * Cette procedure cree un tableau 'tabsym' ou tabsym[i] est un tableau 
 * de longueur tab[i] qui contient les symboles de longueur i+1.
 * Elle lit les symboles directement dans le fichier JPEG.
 */
uint8_t **create_huffman_table(uint32_t tab[], uint32_t *cpt, FILE *f)
{
	uint8_t **tabsym = calloc(sizeof(uint8_t *),16);
	assert(tabsym != NULL);
	int32_t cptcodes = 0;
	for (uint32_t i = 0; i < 16; i++) {
		tabsym[i]=calloc(sizeof(uint8_t),tab[i]);
		assert(tabsym[i] != NULL);
		for (uint32_t j = 0; j < tab[i]; j++) {
			if (cptcodes % 8 == 0)
				printhead("%5u :                 │ ",*cpt);
			tabsym[i][j] = read_byte(cpt,f);
			printhead("%2x ",tabsym[i][j]);
			cptcodes++;
			if (cptcodes % 8 == 0)
				printhead("│\n");
		}
	}
	for (int8_t k = 0; k < (7 - (cptcodes-1) % 8); k++) {
		printhead("   ");
	}
	if (cptcodes % 8 != 0)
				printhead("│\n");	
	printhead("                        └─────────────────────────┘\n");	
	return tabsym;
}


/* Cree un nouveau noeud en l'initialisant avec les parametres */
huff_table_t *new_node(int16_t code, int8_t value, int8_t is_elt, 
			  huff_table_t *parent)
{
	huff_table_t *out = malloc(sizeof(huff_table_t));
	assert(out != NULL);
	out->code = code;
	out->value = value;
	out->is_elt = is_elt;
	out->parent = parent;
	out->left  = NULL;
	out->right = NULL;
	return out;
}


/* 
 * Ajoute une feuille 'symbole' dans l'arbre 'root', a une profondeur 'level'
 * donnee, et le plus a gauche possible.
 * La valeur de retour ne sert qu'a la recursion : 0 si insertion reussie,
 * 1 si echec (niveau plein, ou bien des feuilles empechent de descendre)
 */
uint8_t add_node(uint32_t level, uint8_t symbole, huff_table_t *root)
{
	if (root->is_elt == 1) {
		return 1;
	}
	if (level == 1) {
		if (root->left == NULL) {
			root->left = new_node((root->code)*2,symbole,1,root);
			printhuff("    sym: %u  => code: %u\n",symbole,
				  (root->code)*2);
			return 0;
		} else if (root->right == NULL) {
			root->right = new_node((root->code)*2+1,symbole,1,root);
			printhuff("    sym: %u  => code: %u\n",symbole,
				  (root->code)*2+1);
			return 0;
		} else {
			return 1;
		}
	}
	uint8_t echec;
	if (root->left == NULL) {
		root->left = new_node((root->code)*2,0,0,root);
		echec = add_node(level-1,symbole,root->left);
		assert(echec == 0);
		return 0;
	} else {
		echec = add_node(level-1,symbole,root->left);
	} 
	if (echec == 0) {
		return 0;
	}
	if (root->right == NULL) {
		root->right = new_node((root->code)*2+1,0,0,root);
		echec = add_node(level-1,symbole,root->right);
		assert(echec == 0);
		return 0;
	} else {
		return add_node(level-1,symbole,root->right);
	}
}


/* 
 * Cree l'arbre de Huffman a partir de 'tab' (contenant les nombres de codes
 * par longueur) et de 'tabsym' (contenant les symboles associes)
 * Le resultat est stocke dasn 'root' qui doit etre alloue avant l'appel
 */
void create_huffman_tree(uint32_t *tab, uint8_t **tabsym, huff_table_t *root)
{
	printhuff("Creation d'arbre de huffman\n");
	root->code = 0;
	root->value = 0;
	root->is_elt = 0;
	root->parent = NULL;
	root->left  = NULL;
	root->right = NULL;
	for (uint32_t i = 0; i < 16; i++){
		printhuff("  %u codes de longueur %u :\n",tab[i],i+1);
		for (uint32_t j = 0; j < tab[i]; j++){
			if (add_node(i+1,tabsym[i][j],root)==1){
				printhuff("    erreur interne %u\n",j+1);
			}
		}
	}
}

int32_t load_huffman_table(uint32_t *cpt, FILE *f, huff_table_t *ht)
{
       	printhead("%5u :      nb codes : ",*cpt);
	uint32_t init = *cpt;
	uint32_t tabnb[16];
	for (uint32_t i = 0; i < 16; i++) {
		uint8_t oct = read_byte(cpt,f);
		printhead("%x ",oct);
		tabnb[i] = oct;
	}
	printhead("\n%5u :      symboles : ┌─────────────────────────┐\n",*cpt);
	uint8_t **tabsym = create_huffman_table(tabnb,cpt,f);
	create_huffman_tree(tabnb,tabsym,ht);
	for (uint32_t i = 0; i < 16; i++) {
		free(tabsym[i]);
	}
	free(tabsym);
	return *cpt - init;
}

void free_huffman_tables(huff_table_t *root)
{
	if (root == NULL) {
		return;
	}
	free_huffman_tables(root->left);
	free_huffman_tables(root->right);
	free(root);
}

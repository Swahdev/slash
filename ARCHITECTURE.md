# Structures de données 

Structure EV :
typedef struct EV {
    char* HOME;                 -> Référence du $HOME de la machine 
    char* PWD;                  -> Référence du répertoire courant
    char* MEM_DIR;              -> Précédent PWD

    unsigned int STATUS_CODE;   -> Valeur de retour de la dernière commande exécutée
    char* STATUS_MSG;           -> Possible retour de la dernière commande exécutée
    char* ERROR_MSG;            -> Possible message d'erreur de la dernière commande exécutée
} EV;

# Architecture logicielle

Boucle principale dans la fonction main() du fichier slash.c
-> Génération du prompt puis affichage (extract, parse.c)
-> Parse de l'entrée utilisateur (parse, parse.c)
    -> Reconnaissance de la commande (cmdSearch, parse.c)
    -> Traitement des jokers (gestionDesJokers, gestionDoubleStar, parse.c)
-> Création des tuyaux entre les différentes commandes (parsePipe, parse.c)
    -> Traitement et application des redirections (parseRedirection, parse.c)
    -> Éxecution des différentes commandes (executecmd, parse.c)

Des fonctions auxiliaires pour la manipulation de fichiers se trouvent dans file.c
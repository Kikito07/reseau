/*int main (int argc, char *argv[]){

// variable utile;
    int err;
    fini = false;
    nmb = 0; //nombre de consonne/voyelle
    nmbfiles= 1; //nombre de fichier a traité
    fichier = argv;
    isconsonne = false;
    nthread = 1;
    char options;
    bool isfichierout = false;
    char* fichierout = NULL;
    nbmarg = argc;
    test = 0;

    while((options=getopt(argc, argv,"t:co:v"))!= -1){
         switch(options){

            case 't' : 
            nthread = strtol(optarg,NULL,10);
            break;

            case 'c' :
            isconsonne = true;
            break;

            case 'o' : 
            isfichierout = true;
            fichierout = optarg;
            break;

            case 'v' :
            verbose = true; //permet d'enlever les print pour alléger la console
            break;
        }
    }
}
*/
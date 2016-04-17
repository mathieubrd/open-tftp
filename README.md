# openTFTP

openTFTP est un implémentation du protocole TFTP (voir [RFC 1350](https://tools.ietf.org/html/rfc1350)).

## Compilation
```
make
````
Les exécutables sont placés dans le dossier `bin/`.

## Exécution
Vous devez adapter votre variable d'environnement `LD_LIBRARY_PATH` pour que le programme aille chercher les librairies dans le dosser `lib64`.
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib64
```

Pour lancer le client ou le serveur, placez vous à la racine du projet, puis :
`bin/server` ou `bin/client`

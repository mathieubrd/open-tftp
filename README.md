# openTFTP

openTFTP est un implémentation du protocole TFTP (voir [RFC 2347](https://tools.ietf.org/html/rfc2347)).

## Installation
```
make
````
Les exécutables sont placés dans le dossier `bin/`.
Vous devez adapter votre variable d'environnement `LD_LIBRARY_PATH` pour que le programme puissent trouver les bibliothèques dans le dosser `lib64`.
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib64
```
Vous pouvez aussi déplacez les bibliothèques dans le dossier `/usr/lib64` vous évitant de modifier la variable d'environnement `LD_LIBRARY_PATH`.

## Usage
Tout d'abord, vous devez lancer le serveur. Pour cela, rendez-vous dans la racine du projet et lancez :
```
bin/server
```
>**Attention** ! Si vous avez configuré votre variable `LD_LIBRARY_PATH` comme mentionné ci-dessus, vous devez lancer les executables depuis la racine du projet.

Pour lancer le client, rendez-vous à la racine du projet et lancez :
```
bin/client <fichier> <destination>
```
`<fichier>` correspond au nom du fichier stocké sur le serveur.
`<destination>` correspond au chemin où sera écrit le fichier téléchargé.

**Exemple :** `bin/client image.jpg ~/Images/photo.jpg`

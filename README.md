# openTFTP

openTFTP est un implémentation du protocole TFTP (voir [RFC 2347](https://tools.ietf.org/html/rfc2347)).

## Installation
```
make
````
Les exécutables sont placés dans le dossier `bin/`.

## Usage
Tout d'abord, vous devez lancer le serveur. Pour cela, rendez-vous dans la racine du projet et lancez :
```
bin/server <port>
```
Où `port` est le port d'écoute du client.

Pour lancer le client, rendez-vous à la racine du projet et lancez :
```
bin/client <adresse> <port> <fichier> <destination>
```
`<adresse>` correspond à l'adresse du serveur (sous forme d'IP ou de nom d'hôte).
`<port>` correspond au port d'écoute du serveur.
`<fichier>` correspond au nom du fichier stocké sur le serveur.
`<destination>` correspond au chemin où sera écrit le fichier téléchargé.

**Exemple :** `bin/client 192.168.1.54 25565 image.jpg ~/Images/photo.jpg`

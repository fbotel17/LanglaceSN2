# Le Projet GPS

Le projet GPS est une collaboration entre **Joris**, **Faustin** et **Edouard**, ayant pour but d'offrir une expérience utilisateur optimale dans le domaine de la géolocalisation.

## 🌐 Adresses IP des Machines Virtuelles 

Les machines virtuelles dédiées au projet sont accessibles via les adresses IP suivantes:
- **Site Web** : `192.168.65.68`
- **Base de Données** : `192.168.65.252`

## 🗃 Base de Données : Lawrence 

Pour accéder à cette base de données, voici les identifiants:
- **Identifiant** : `root`
- **Mot de passe** : `root`

### Structure de la base de données

**Lawrence**

**Table : user**

| Champ     | Type           | Spécificité          |
|-----------|----------------|----------------------|
| idUser    | int            | Clé primaire         |
| nom       | varchar(30)    |                      |
| email     | varchar(300)   |                      |
| password  | varchar(30)    |                      |
| isAdmin   | tinyint(1)     |                      |

**Table : GPS**

| Champ             | Type                                                                |
| ----------------- | ------------------------------------------------------------------ |
| IdBateau | int(11) |
| date | date |
| heure | time |
| latitude | varchar(100) |
| longitude | varchar(100) |
| vitesse | int(11) |
| vitesseMoyenne | int(11) |




## 📁 Structure du Code

Les fichiers et répertoires sont organisés comme suit:

  - `main.cpp` : Dans cette page se situe tout le code pour la reception des trames GPS.


### Fichiers Principaux :

- `readme.md` : Documentation du code (ce fichier).

> **Conseil** : Pour une meilleure compréhension du projet, n'hésitez pas à parcourir chaque fichier et répertoire.

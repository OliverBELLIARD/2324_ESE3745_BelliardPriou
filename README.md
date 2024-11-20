# 2324_ESE3745_BelliardPriou
> Noms : Oliver BELLIARD & Valérian PRIOU  
> Encadrant : Alexis MARTIN  

## 1. Objectifs
A partir d'un hacheur complet et d'une carte Nucleo-STM32G474RE, vous devez :
- Réaliser un shell pour commander le hacheur,
- Réaliser la commande des 4 transistors du hacheur en commande complémentaire décalée,
- Faire l'acquisition des différents capteurs,
- Réaliser l'asservissement en temps réel.
  
Les livrables demandés :
- Tout le projet stm32cubeIDE synchronisé sur un github dès la première séance de TP,
- Un fichier readme du Github reprenant l'ensemble des éléments demandés (nom obligatoire : 2324_ESE3745_<Nom1><Nom2>),
- Une documentation réalisée pour toutes les fonctions écrites avec Doxygen.  
  
Votre projet github doit être public ou partagé avec votre professeur (nicolas.papazoglou@ensea.fr ou alexis.martin@ensea.fr).  
  
L'évaluation aura lieu tout au long des séances de TP sur plusieurs points :
- Avancement dans le sujet,
- Aptitude a réutiliser les notions vue en cours,
- Habilité à aller chercher dans la documentation les informations utiles,
- Qualité de la documentation
- Lisibilité du code et qualité du code (mise en place de fonctions, factorisation du code, choix des noms de variables/fonctions explicites)
- Présence et ponctualité,
  
## 2. Bonnes pratiques

Pour une meilleure lisibilité du code :
- Toutes les variables sont écrites en minuscule avec une majuscule pour les premières lettre à partir du 2eme mot dans le nom de la variable, par exemple : uartRxBuffer
- Les directives (macro avec #define) sont écrites en majuscule, par exemple : #define UART_TX_SIZE
- Aucun nombre magique ne peut exister, par exemple : remplacer if(value == 512) par if(value == VAL_MAX) avec #define VAL_MAX 512
- Si vous écrivez deux fois le même code, il faut écrire une fonction pour "factoriser" votre code.
- Il faut tester vos appels de fonction, les fonctions HAL renvoient pour la pluspart une valeur pour savoir si son éxécution s'est déroulé correctement.
    
## 3. Github et Doxygen
Installer Doxygen et faire une première documentation (https://doxygen.nl/index.html)  
> To launch Doxygen GUI, `doxywizard` must be typed in the terminal.

## 4. PCB
Tout le projet Kicad de la cartes est disponible dans l'archive Inverter_Kicad_Project.  
Inverter Projet Kicad : https://github.com/DBXYD/AAP_ENSEA_Inverter/tree/master

## 5. Console UART

Dans un premier temps nous allons paramétrer la liaison UART de la carte NUCLEO-G474RE.  
  
Plusieurs fonctionnalités sont déjà codées :
- La présence d'un écho pour tous les caractères pour voir ce que l'on envoie.  
- Lorsque le caractère "ENTER" est détecté, traiter la chaine de caractères en la comparant aux commandes connues, pour le moment nous resterons à un nombre limitée de commandes.  
  
Les fonctions déjà présentes fournissent le couple de valeur "argc" "argv[]" communément connu lorsque vous codez sur PC.
Vous allez devoir au cours du TP (pas maintenant), coder les fonctions suivantes et les maintenant à jour :
- help : qui affiche toutes les commandes disponibles,
- start : qui allume l'étage de puissance du moteur (pour l'instant nous ne ferons qu'afficher un message de "power on" dans la console)
- stop : qui éteind l'étage de puissance du moteur (pour l'instant nous ne ferons qu'afficher un message de "power off" dans la console)
- tout autre fonction qui vous semble nécessaire
- toute autre commande renverra un message dans la console "Command not found"

## 6. Séance 1 - Commande MCC basique

Commande de MCC - niveau basique

Objectifs :
- Générer 4 PWM en complémentaire décalée pour contrôler en boucle ouverte le moteur en respectant le cahier des charges,
- Inclure le temps mort,
- Vérifier les signaux de commande à l'oscilloscope,
- Prendre en main le hacheur,
- Faire un premier essai de commande moteur.

### 6.1. Génération de 4 PWM

Générer quatre PWM sur les bras de pont U et V pour controler le hacheur à partir du timer déjà attribué sur ces pins.  

Cahier des charges :
- **Fréquence de la PWM : 20kHz** : Nous avons configurés le Timer1 en "Center Aligned Mode 1" (centré avec comptage et décomptage), ce qui veut dire que la fréquence que nous avons calculés doit prendre en compte la division par 2 sur la fréquence entrainée par cette configuration. Nous avons donc configurés notre période de comptage (AutoReload Register, *ARR*) à **4250-1**. Ce qui nous donne :  
    170 000 000 / (2\*PSC\*ARR) = 170 000 000 / (2\*1\*(1+4250-1)) = 20 000 Hz  
- **Temps mort minimum : à voir selon la datasheet des transistors (faire valider la valeur)** : Sur la maquette, nous trouvons le MOSFET de puissance IRF540NPbF, dans sa documentation présente dans le GitHub : [AAP_ENSEA_Inverter/datasheets/IRF540NPbF.pdf](https://github.com/DBXYD/AAP_ENSEA_Inverter/blob/master/datasheets/IRF540NPbF.pdf) nous observons les temps d'action des transistors :  
    ![image](https://github.com/user-attachments/assets/670d02d1-0eef-4e65-9f36-6d6f07bfbfe8)  
    On a alors un temps de monté total de : **td(on) + tr = 11 + 35 = 46 ns** et un temps de descente total de : **td(off) + tf = 39 + 35 = 74 ns**. Nous devons prendre en compte le pire des cas, le temps de descente total pour choisir un temps mort qui soit au dessus. Nous avons alors choisi, **200 ns > 74 ns**, ce qui correspond à :  
    Temps mort souhaité * Fréquence le l'horloge du timer / PSC = 200\ * 10⁻⁹\*170\*10⁶ / 1 = **34 unités** dans la configuration "Dead Time" du Timer1.  

![temps_mort](https://github.com/user-attachments/assets/17bbb334-58f0-4a34-b710-eb53d945609a)
  
- **Résolution minimum : 10bits** : Nous avons besoin d'une valeur de comptage supérieure à 2¹⁰ = 1024. C'est pourquoi, en choisissant une valeur d'ARR = 4250-1 > 1024 nous respectons le cahier des charges.
  
Pour les tests, fixer le rapport cyclique à 60%.  
Une fois les PWM générées, les afficher sur un oscilloscope et les faire vérifier par votre professeur.  

  ![tek00006](https://github.com/user-attachments/assets/bb5c0ea3-d72b-4e38-a1f7-1b7a16e0b04d)
  
### 6.2. Commande de vitesse

Pour controler la vitesse du moteur, nous allons envoyer une séquence via la liaison UART (par l'USB) de la forme :  
  
speed XXXX
  
Le traitement de cette chaine de caractère se faire de la manière suivant :
- Détection des premiers caractères "speed"
- Conversion de tous les caractères représentant des chiffres XXXX en nombre entier
- Vérification de la valeur (si la valeur est supérieur au maximum autorisé (bien spécifier cette valeur), on l'abaisse à cette valeur),
- Application de cette vitesse au moteur à travers le registre gérant le rapport cyclique de la PWM
  
Nous avons créé une fonction pour configurer la commande décalée des PWM facilement tel que ci dessous.

```c
/**
 * @brief	Sets the offset PWM for all channel of TIM1.
 * @param	int	pulse to apply.
 */
void set_PWM(int pulse)
{
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, __HAL_TIM_GET_AUTORELOAD(&htim1) - pulse);
}
```

C'est cette fonction que nous appelons à chaque fois que nous modifions les rapports cycliques. Ainsi, la fonction appelé par la commande `speed` ressemble à ceci :
```c
/**
 * @brief	Sets the PWM for a channel of TIM1.
 * @return	int	Desired PWM duty pulse (0 to PWM_MAX_VAL).
 */
void set_PWM_speed(int speed)
{
	if (speed < PWM_MAX_VAL && speed > 0)
	{
		// Set main PWM pulse width for Channel 1 and Channel 2
		set_PWM(speed);
	}
}
```

### 6.3. Premiers tests

Brancher le moteur en respectant les données constructeur du moteur.  
  
Faites vérifier l'ensemble des signaux par votre professeur.  
  
Faire un premier test dans les conditions suivantes (dans l'ordre) :
- Rapport cyclique de 50% : ![alpha05](https://github.com/user-attachments/assets/29c9a350-cccc-4fc5-8c8c-b00f27681c4e)

- Rapport cyclique de 70% : ![tek00010](https://github.com/user-attachments/assets/59552fd3-ca09-4721-ad08-95ff67e02ee3)

  
Quels problèmes observez vous ?  
  
Pour palier à ce problème, générer une montée progressive du rapport cyclique jusqu'à arriver à la vitesse cible commandé par la commande définie précédemment.  

```c
/**
 * @brief	Sets the offset PWM for all channel of TIM1 at a defined rate (PWM_VARIATION_RATE) recursively.
 * @param	int	Pulse to apply.
 * @attention	Global variable current_speed_PWM must be initialised.
 */
void set_PWM(int pulse)
{
	if (pulse < current_speed_PWM)
	{
		current_speed_PWM -= 1;
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
				current_speed_PWM);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
				__HAL_TIM_GET_AUTORELOAD(&htim1) - current_speed_PWM);

		//printf("Réduction de la vitesse\r\n");
		HAL_Delay(PWM_VARIATION_RATE);
		set_PWM(pulse);
	}
	else if (pulse > current_speed_PWM)
	{
		current_speed_PWM += 1;
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
				current_speed_PWM);
		__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
				__HAL_TIM_GET_AUTORELOAD(&htim1) - current_speed_PWM);

		//printf("Augmentation de la vitesse\r\n");
		HAL_Delay(PWM_VARIATION_RATE);
		set_PWM(pulse);
	}
}
```
Cette méthode, plus efficace avec des interruptions, permet de faire varier la vitesse à l'aide du CPU. Une optimisation est de dédier cette variation à une fonction par interruption avec un Timer.

## 7. TP n°2 - Commande en boucle ouverte, mesure de Vitesse et de courant

Dans cette partie vous devez :

- Commander en boucle ouverte le moteur avec une accélération limitée,
- Mesurer le courant aux endroits adéquat dans le montage,
- Mesurer la vitesse à partir du codeur présent sur chaque moteur.

### 7.1. Commande de la vitesse

Rajouter quelques fonctionnalités à votre projet :
  
- Commande start : permet de fixer le rapport cyclique à 50% (vitesse nulle) et d'activer la génération des pwm (HAL_TIM_PWM_Start et HAL_TIMEx_PWMN_Start),
  ```c
  
- Commande stop : permet de désactiver la génération des PWM.
- Commande speed XXXX : permet de définir le rapport cyclique à XXXX/PWM_MAX, mais afin de réduire l'appel à courant, vous devez établir une montée progressive à cette vitesse en quelques secondes. Vous pouvez effectuer une rampe entre la valeur actuelle et la valeur cible avec un incrément bien réfléchi de la PWM à un intervalle de temps régulier. Par la suite votre asservissement fera cela tout seul.

### 7.2. Mesure du courant

A partir de la documentation (schéma KiCad) :
  
- Définir quel(s) courant(s) vous devez mesurer,
- Définir les fonctions de transfert des capteurs de mesure de courant (lecture datasheet),
- Déterminer les pin du stm32 utilisés pour faire ces mesures de courant,
- Etablir une première mesure de courant avec les ADC en Pooling. Faites des tests à vitesse nulle, non nulle, et en charge (rajouter un couple resistif en consommant du courant sur la machine synchrone couplée à la MCC).
- Une fois cette mesure validée, modifier la méthode d'acquisition de ces données en établissant une mesure à interval de temps régulier avec la mise en place d'une la chaine d'acquisition Timer/ADC/DMA.
- Vous pouvez utiliser le même timer que celui de la génération des PWM pour que les mesures de courant soit synchrone aux PWM. Pour vérifier cela, utiliser un GPIO disponible sur la carte pour établir une impulsion lors de la mesure de la valeur.
  
![tek00009](https://github.com/user-attachments/assets/0ce8823a-54de-4afb-9af6-251b9874e68d)

### 7.3. Mesure de vitesse
A partir de la documentation (schéma KiCad, datasheets et expérimentation) :

- Déterminer la fonction de transfert du capteur de vitesse,
- Déterminer la constant de temps mécanique du moteur :  
	Pour cela, vous pouvez envoyer un échelon de tension au moteur et analyser la vitesse à partir de la sonde tachymétrique (que l'on n'utilisera pas pour le reste du TP).
- Déterminer les pin du stm32 utilisés pour faire cette mesure de vitesse,
- Déterminer la fréquence à laquelle vous allez faire l'asservissement en vitesse du moteur.
- Etablir le code de mesure de vitesse et le tester.

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
#define PWM_VARIATION_RATE 3

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
Cette méthode qui pourrait être plus efficace avec des interruptions, permet de faire varier la vitesse à l'aide du CPU. Une optimisation est de dédier cette variation à une fonction par interruption avec un Timer pour que la variation progressive soit non bloquante.

## 7. TP n°2 - Commande en boucle ouverte, mesure de Vitesse et de courant

Dans cette partie vous devez :

- Commander en boucle ouverte le moteur avec une accélération limitée,
- Mesurer le courant aux endroits adéquat dans le montage,
- Mesurer la vitesse à partir du codeur présent sur chaque moteur.

### 7.1. Commande de la vitesse

Rajouter quelques fonctionnalités à votre projet :
  
- Commande start : permet de fixer le rapport cyclique à 50% (vitesse nulle) et d'activer la génération des pwm (`HAL_TIM_PWM_Start` et `HAL_TIMEx_PWMN_Start`) :
	```c
	/**
	* @brief Starts PWM on TIM1 channels. Initial base speed set to 0.5 duty cycle.
	*/
	void start_PWM()
	{
		// TIM1 Channel 1 Initialisation
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

		// TIM1 Channel 2 Initialisation
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

		current_speed_PWM = (int)(PWM_MAX_VAL/2)+1; // We initialize the base speed to 0 (cyclic rate 0.5)
		set_PWM_ratio(0.5);
	}
	```
- Commande stop : permet de désactiver la génération des PWM :
	```c
	/**
	* @brief Stops PWM on TIM1 channels.
	*/
	void stop_PWM()
	{
		// We disable Tim1 channel 1
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);

		// We disable Tim1 channel 2
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
	}
	```
- Commande speed XXXX : permet de définir le rapport cyclique à XXXX/PWM_MAX, mais afin de réduire l'appel à courant, vous devez établir une montée progressive à cette vitesse en quelques secondes. Vous pouvez effectuer une rampe entre la valeur actuelle et la valeur cible avec un incrément bien réfléchi de la PWM à un intervalle de temps régulier. Par la suite votre asservissement fera cela tout seul.
Nous avons modifié notre variation progressive en utilisant un timer à la place de la récursivité pour avoir un changement de vitesse non bloquant :
	1. Nous avons alors utilisé le TIM2 en activant ses interruptions périodiques.
	2. Ensuite nous appelons la fonction qui compare la vitesse courrante à une consigne pour l'ajuster (`set_PWM()`) dans la fonction de Callback.
	3. Notre code ressemble alors à ce ci :
	```c
	int current_speed_PWM;
	int requested_speed_PWM;

	/**
	* @brief Adjusts PWM pulse for TIM1 channels.
	* @param pulse Desired PWM pulse width
	* @attention The global variables current_speed_PWM and requested_speed_PWM must be initialized.
	*/
	void set_PWM()
	{
		if (requested_speed_PWM < current_speed_PWM)
		{
			current_speed_PWM -= 1;
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
					current_speed_PWM);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
					__HAL_TIM_GET_AUTORELOAD(&htim1) - current_speed_PWM);
		}
		else if (requested_speed_PWM > current_speed_PWM)
		{
			current_speed_PWM += 1;
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1,
					current_speed_PWM);
			__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2,
					__HAL_TIM_GET_AUTORELOAD(&htim1) - current_speed_PWM);
		}
	}

	/**
	* @brief Sets the PWM duty cycle ratio for TIM1 channels.
	* @param ratio Duty cycle ratio (0.0 to 1.0) with 12-bit resolution
	*/
	void set_PWM_ratio(double ratio)
	{
		if (ratio < 1 && ratio > 0)
		{
			// Set main PWM pulse width for Channel 1 and Channel 2
			requested_speed_PWM = (int)(ratio * PWM_MAX_VAL);
		}
	}

	/**
	* @brief Sets a specific PWM pulse width for TIM1 channels.
	* @param speed Desired PWM pulse width (0 to PWM_MAX_VAL)
	*/
	void set_PWM_speed(int speed)
	{
		if (speed < PWM_MAX_VAL && speed > 0)
		{
			// Set main PWM pulse width for Channel 1 and Channel 2
			requested_speed_PWM = speed;
		}
	}

	/**
	* @brief  Period elapsed callback in non blocking mode
	* @note   This function is called  when TIM6 interrupt took place, inside
	* HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
	* a global variable "uwTick" used as application time base.
	* @param  htim : TIM handle
	* @retval None
	*/
	void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{
		/* USER CODE BEGIN Callback 0 */

		if (htim->Instance == TIM2) {
			set_PWM();
		}

		/* USER CODE END Callback 0 */
		if (htim->Instance == TIM6) {
			HAL_IncTick();
		}
		/* USER CODE BEGIN Callback 1 */

		/* USER CODE END Callback 1 */
	}
	```
	4. Ce sont les seuls changement nécessaires car les commandes appellent ces fonctions pour faire varier la vitesse.

### 7.2. Mesure du courant

A partir de la documentation (schéma KiCad) :
  
- Définir quel(s) courant(s) vous devez mesurer,
- Définir les fonctions de transfert des capteurs de mesure de courant (lecture datasheet),
- Déterminer les pin du stm32 utilisés pour faire ces mesures de courant,
- Etablir une première mesure de courant avec les ADC en Pooling. Faites des tests à vitesse nulle, non nulle, et en charge (rajouter un couple resistif en consommant du courant sur la machine synchrone couplée à la MCC).
- Une fois cette mesure validée, modifier la méthode d'acquisition de ces données en établissant une mesure à interval de temps régulier avec la mise en place d'une la chaine d'acquisition Timer/ADC/DMA.
- Vous pouvez utiliser le même timer que celui de la génération des PWM pour que les mesures de courant soit synchrone aux PWM. Pour vérifier cela, utiliser un GPIO disponible sur la carte pour établir une impulsion lors de la mesure de la valeur.
  
Nous voulons mesurer le courant U_Imes, accessible grâce à la mesure du Arm_IRF540NBbF, numérisé par l'ADC1, branché au pin PA1.

Pour mesurer le courrant en Pooling dans un premier temps nous avons ajouté une commande `current` qui appelle la fonction `read_current()` que nous avons codé de la façon suivante :
```c
/**
 * @brief Reads the current U_Imes.
 */
void read_current()
{
	uint32_t adc_value = 0;
	double current = 0;
	double tension = 0;

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	adc_value = HAL_ADC_GetValue(&hadc1);

	printf("\r\nRAW adc: %d\r\n", (int)adc_value);

	// Convertion taking into account the offset due to the unsigned ADC measure
	/**
	 * Convertion taking into account the offset due to the unsigned ADC measure:
	 * 	Sensor resolution: 50 mV/A
	 * 	Vout = 3.3/2 + 0.05*Imeasured
	 **/
	tension = ADC_VCC * ((int)(adc_value) - ADC_OFFSET) / ADC_MAX_VAL;
	current = (tension) / ADC_CURRENT_RESOLUTION;

	printf("\r\nMeasured tension: %f V\r\n", tension);
	printf("\r\nMeasured current: %f A\r\n", current);
}
```
En ajoutant la calibration du ADC1 dans la fonction `main()` :
```c
HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
```
Avec cette méthode nous observons des mesures quasiment aléatoires dans les valeurs de courant mesurées. Cela est du à notre courant qui fluctue beaucoup avec les PWM. Nous devons mesurer périodiquement, au même rythme que les PWM. Pour celà nous pouvons utiliser le DMA, avec des mesures déclenchés par le timer 1, celui qui génère les PWM. En configurant le TIM1 pour :  
	- `Master/Slave Mode (MSM bit)` : Enable  
	- `Trigger Event Selection TRGO` : Update Event  
Et l'ADC1 pour :  
	- `Enable Regular Conversions` : Enable  
	- `External Trigger Conversion Source` : Timer 1 Trigger Out Event 
  
Notre nouveau code pour lire les mesures avec la commande `current` ressemble alors à ce ci :
```c
#define ADC_CURRENT_RESOLUTION 0.05
#define ADC_VCC 3.3
#define ADC_MAX_VAL 4096
#define ADC_OFFSET 2421
#define ADC_BUFF_SIZE 1

uint32_t pData[ADC_BUFF_SIZE];

double U_Imes = 0;
double Uadc = 0;

/**
 * @brief Reads the current U_Imes.
 */
void read_current()
{
	// Read ADC1 DMA to update pData
	printf("\r\nRAW ADC value: %d\r\n", (int)(pData[0]));

	/**
	 * Convertion taking into account the offset due to the unsigned ADC measure:
	 * 	Resolution: 50 mV/A
	 * 	Vout = 3.3/2 + 0.05*Imeasured
	 **/
	Uadc = ADC_VCC * ((int)(pData[0]) - ADC_OFFSET) / ADC_MAX_VAL;
	U_Imes = (Uadc) / ADC_CURRENT_RESOLUTION;

	printf("\r\nMeasured tension: %f V\r\n", Uadc);
	printf("\r\nMeasured current: %f A\r\n", U_Imes);
}

/**
 * @brief  Conversion complete callback in non-blocking mode. Updates the data read from the DMA.
 * @param hadc ADC handle
 * @retval None
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc->Instance == ADC1)
		HAL_ADC_Start_DMA(hadc, pData, ADC_BUFF_SIZE);
}
```
Sans oublier d'inclure dans la fonction `main()` :
```c
HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
HAL_ADC_Start_DMA(&hadc1, pData, ADC_BUFF_SIZE);
```
  
Avec cette nouvelle méthode, nous avons des mesures de courrant plus cohérentes et il est cette fois-ci possible d'observer les variations de courrant en faisant varier le rapport cyclique ou la charge du moteur.  
Il est à noter que nous avons observé un offset de courrant quand le rapport cyclique valait 0.5. D'où la constante `ADC_OFFSET` pour ramener la mesure du courrant à zéro au rapport cyclique 0.5.  
  
Cet écart est observable sur la capture d'oscilloscope ci dessous :  
  
![tek00009](https://github.com/user-attachments/assets/0ce8823a-54de-4afb-9af6-251b9874e68d)
  
On observe une tension de 1.59 V issue de la sonde de tension sur PA1. Ce qui nous donne une valeur d'ADC 4096/1.56 = 2626, proche du `ADC_OFFSET` mesuré.
  
### 7.3. Mesure de vitesse
A partir de la documentation (schéma KiCad, datasheets et expérimentation) :

- Déterminer la fonction de transfert du capteur de vitesse,
- Déterminer la constant de temps mécanique du moteur :  
	Pour cela, vous pouvez envoyer un échelon de tension au moteur et analyser la vitesse à partir de la sonde tachymétrique (que l'on n'utilisera pas pour le reste du TP).
- Déterminer les pin du stm32 utilisés pour faire cette mesure de vitesse,
- Déterminer la fréquence à laquelle vous allez faire l'asservissement en vitesse du moteur.
- Etablir le code de mesure de vitesse et le tester.

Nous pouvons mesurer la vitesse grâce à l'encodeur présent sur le Moteur.

## 8. TP n°3 Asservissement
Dans cette partie vous devez :
- Etablir l'asservissement en vitesse du moteur,
- Etablir l'asservissement en courant du moteur.
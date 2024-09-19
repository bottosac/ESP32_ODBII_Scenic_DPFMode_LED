Lettura del messaggio CAN 222056 del motore 1.5dci Renault. La lettura restituisce un numero che ha i seguenti valori
1=normale
5=rigenerazione
7=attesa
Nel progetto Arduino è necessario impostare il MAC Address del dispositivo ELM327. Si può ottenerlo facilmente collegandosi da cellulare e andando a vedere il dispositivo bluetooth associato.
Quando ELM327 è collegato si accende il led blu. Ogni volta che la scheda interroga via OBD il dispositivo lampeggia di rosso.
Quando il valore letto è diverso da 1 si accende il led rosso fisso.
Quando il motore è arrestato il valore passa a 7 per alcuni secondi.
E' necessario installare la libreria ELMduino tramite il gestore librerie del Arudino IDE o in alternativa scaricarlo a questo indirizzo https://github.com/PowerBroker2/ELMduino

06/09/2024: v1 lettura messaggio 22 2056 DPF Mode (1=normale, 5=rigenerazione, 7=attesa) e gestione tramite led

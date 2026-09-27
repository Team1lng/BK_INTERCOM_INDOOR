#include "layout_define.h"
#include "../api/queue/queue.h"
#include "ak_thread.h"
#include "user_data.h"
#include "tuya_uuid_and_key.h"
#include "tuya_sdk.h"

#define OS_EVENT_NUM_MAX 32

typedef enum
{
	OS_EVENT_TYPE_NONE,
	OS_EVENT_TYPE_RECORD,
	/*
	 * arg1:sd状态类型,:1.拔插 2.格式化，3.sdcar内存状态
	 * arg2:arg1= 1,arg2= 0 :正在扫描，1:扫描完成
	 *	   arg1=2,arg2=1:开始格式化，2:格式化完成。3:格式出错
	 *	   arg1=3,arg2=1:内存正常，2，内存已满
	 */
	OS_EVENT_TYPE_SD,

	/*arg1:重复的ID号*/
	OS_EVENT_TYPE_DEVICE_REPEAT,

	OS_EVENT_TYPE_INTERPHONE,

	/* 铃声播放回调 arg1:函数入口，arg2:1:起始,arg2:2结束*/
	OS_EVENT_TYPE_RING,

	/*arg1 :arg1 = DEVICE_OUTDOOR_1 or DEVICE_OUTDOOR_2*/
	OS_EVENT_TYPE_OUTDOOR_CALL,

	/*
	*	arg1(高8位,低8位预留):事件类型:1:其他设备要通话。arg2:高8位表示室内机，低8位表示门口机 (此事件上传到主线程，一般处理原则，退出监控页面，
		但是此设备正与其他户外机通话状态下则不做任何处理)
	*
	*					2:表示此命令正忙，需要等待,低8位:正忙的详细信息。arg2:高8位代表室内机，低8位 代表是户外机
	*
	*/
	OS_EVENT_TYPE_MOTION_DETECT,
	OS_EVENT_TYPE_TUYA,
	OS_EVENT_TYPE_INDOOR_CMD,
	OS_EVENT_TYPE_OUTDOOR_STATUS,

	OS_EVENT_TYPE_WEATHER_STATUS,

	OS_EVENT_TYPE_UPGRADE,

	OS_EVENT_TYPE_CHIME,

	OS_EVENT_TYPE_MAILBOX,

	OS_EVENT_TYPE_TCP_NETWORK,

	OS_EVENT_TYPE_MECHANICAL_KEY,

	OS_EVENT_TYPE_ALARM,

	OS_EVENT_TYPE_MONITOR_BUSY,

	OS_EVENT_TYPE_GATE2_UNLOCK,

	OS_EVENT_TYPE_ADC_KEY_UNLOCK,
} event_type;

typedef struct
{
	event_type type;
	unsigned long arg1;
	unsigned long arg2;
	unsigned long arg3;

} event_msg;

typedef struct
{
	void *prev;
	void *next;

	event_msg msg;

} lv_event_info;

static event_pro_callback snap_event_callback = NULL;
static event_pro_callback record_video_event_callback = NULL;
static event_pro_callback sd_event_callback = NULL;

static void device_id_repeat_callback_default(unsigned long arg1, unsigned long arg2);
static event_pro_callback device_id_repeat_callback = device_id_repeat_callback_default;

static event_pro_callback interphone_call_callback = NULL;

static event_pro_callback monitor_call_callback = NULL;
static event_pro_callback upgrade_callback = NULL;

static event_pro_callback motion_detect_callback = NULL;

static event_pro_callback indoor_cmd_callback = NULL;
static event_pro_callback network_event_callback = NULL;
static event_pro_callback tuya_event_callback = NULL;

static event_pro_callback info_status_callback = NULL;

static event_pro_callback weather_status_callback = NULL;

static event_pro_callback door_chime_callback = NULL;

static event_pro_callback mechanical_key_callback = NULL;

static event_pro_callback alarm_callback = NULL;

static event_pro_callback mailbox_status_callback = NULL;

static tcp_event_pro_callback tcp_network_callback = NULL;

static event_pro_callback device_monitor_busy_callback = NULL;

static event_pro_callback device_gate2_unlock_callback = NULL;

static event_pro_callback device_adc_key_callback = NULL;

static const layout *cur_layout = NULL;
static const layout *prev_layout = NULL;

static lv_task_t *lv_os_event_ptask = NULL;

static queue_s lv_os_event_queue_head;
static queue_s lv_os_event_queue_free;

static ak_mutex_t lv_os_event_queue_head_mutex;
static ak_mutex_t lv_os_event_queue_free_mutex;

static lv_event_info lv_os_event_msg_buffer[OS_EVENT_NUM_MAX];

#ifdef BCOM_OID_VERSION
const char *multi_lingual[STR_TOTAL][LANGUAGE_TOTAL] =
	{
		{"English", "中文", "Deutsch", "עברית", "Polski", "Portuguese", "Spanish", "French", "日本語", "Italian"},
		{"Intercom", "转呼", "Interner Anruf", "אינטרקום", "Interkom", "Intercomunicador", "Intercomunicador", "Intercom", "内線通話", "Interfono"},
		{"Monitoring", "监控", "Türstation", "צג", "Podgląd", "Visualização", "Vigilancia", "Visualisation", "カメラ視聴", "Monitoraggio"},
		{"Video", "视频", "Video", "וידאו", "Wideo", "Vídeo", "Video", "Vidéo", "ビデオ", "Video"},
		{"Event", "事件", "Veranstaltung", "ארוע", "Dziennik", "Evento", "Evento", "Événements", "記録済み", "Evento"},
		{"Media", "多媒体", "Medien", "קבצים", "Multimedia", "Midia", "Multimedia", "Multimédia", "マルチメディア", "Media"},
		{"Setting", "设置", "Einstellungen", "הגדרה", "Ustawienia", "Ajustes", "Ajuste", "Paramètres", "設定", "Impostazioni"},
		{"At home", "在家", "Anwesend", "בבית", "W domu", "Em casa", "En casa", "Mode maison", "在宅モード", "In casa"},
		{"Not at home", "离家", "Abwesend", "לא בבית", "Poza domem", "Não estou em casa", "No estoy en casa", "Mode absent", "外出モード", "Non a casa"},
		{"Dormant", "休眠", "Ruhend", "מצב שינה", "Noc", "Dormindo", "Dormindo", "Mode nuit", "就寝モード", "Dormiente"},
		{"NULL", "开锁", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL"},
		{"Lock", "Lock", "Schleuse", "מנעול", "Zamek", "Fechadura", "Cerrar", "Serrure", "玄関錠", "Serrare"},
		{"Gate", "Gate", "Gate", "שער", "Brama", "Portão", "Puerta", "portail ", "門扉", "Porta"},
		{"Gate1", "Gate1", "Gate1", "שער1", "Brama1", "Portão1", "Puerta1", "portail ", "門扉1", "Porta1"},
		{"Gate2", "Gate2", "Gate2", "שער2", "Brama 2", "Portão2", "Puerta2", "Relais int.", "門扉2", "Porta2"},
		{"Standby", "待机", "Standby", "המתנה", "Czuwanie", "Em Espera", "Apoyar", "Veille", "スタンバイ", "Stand-by"},
		{"Please insert SD card", "请插入SD卡", "Bitte SD-Karte einlegen", "אנא הכנס כרטיס SD", "Proszę włożyć kartę SD", "Insira o cartão SD", "Inserte la tarjeta SD", "Veuillez insérer une carte SD", "SDカードを挿入してください", "Inserisci la scheda SD"},
		{"No SD card", "无SD卡", "keine SD-Karte", "אין כרטיס SD", "brak karty SD", "Nenhum cartão SD", "Sin tarjeta SD", "Pas de carte SD", "SDカードがありません", "Nessuna scheda SD"},
		{"SD card inserted", "已插SD卡", "SD-Karte eingelegt", "כרטיס SD הוכנס", "Włożona karta SD", "Cartão SD inserido", "Tarjeta SD insertada", "Carte SD insérée", "SDカードが挿入されました", "Scheda SD inserita"},
		{"Please format SD card", "请格式化SD卡", "Formatieren Sie Die karte", "אנא תצור את כרטיס SD", "Formatuj kartę", "Formate por favor o cartão SD", "Por favor, formatear la tarjeta SD", "Veuillez Formater la carte SD", "SDカードをフォーマットしてください", "Formattare scheda SD"},
		{"Yes", "是", "Ja", "כן", "Tak", "Sim", "Sí", "Oui", "はい", "SÌ"},
		{"No", "否", "Nein", "לא", "Nie", "Não", "No", "Non", "いいえ", "NO"},
		{"IDconflict", "ID冲突", "ID-Konflikt", "התנגשות כתובות", "ID konflikt", "Conflito de ID", "Conflicto de identificación", "Conflit ID ", "機器IDが競合しています", "IDconflitto"},
		{"Local ID", "本地ID", "Local ID", "ID מקומי", "Lokalny ID", "ID local", "ID local", "ID local", "自分のID", "ID locale"},
		{"Called ID", "被呼ID", "ID genannt", "נקרא ID", "Nazywany ID", "ID chamado", "ID llamado", "ID Appelant", "相手のID", "ID chiamato"},
		{"call timeout", "呼叫超时", "Anrufzeitüberschreitung", "פסק זמן של שיחה", "Połączenie koniec czasu", "Tempo chamada expirado", "Tiempo llamada expirado", "Durée appel", "呼び出し時間切れ", "Timeout chiamata"},
		{"wait timeout", "等待超时", "Zeitüberschreitung warten", "זמן שיחה הסתים", "czas oczekiwania", "Espere o tempo limite", "tiempo de espera de espera", "Délai d'attente", "待機時間切れ", "tempo di attesa"},
		{"call timed out", "通话超时", "Anruf abgelaufen", "זמן המתנה הסתים", "Limit czasu interkomu", "Chamada cronometrada", "Llame al tiempo de expulsión", "Délai appel dépassé ", "通話時間切れ", "chiamata scaduta"},
		{"Door1", "门口机1", "Türstation 1", "דלת  1", "Drzwi 1", "Porta1", "Puerta1", "Platine 1", "玄関ドアフォン1", "Porta1"},
		{"Door2", "门口机2", "Türstation 2", "דלת  2", "Drzwi 2", "Porta2", "Puerta2", "Platine 2", "玄関ドアフォン2", "Porta2"},
		{"Camera1", "摄像机1", "Kamera 1", "מצלמה 1", "Kamera 1", "Câmera1", "Cámara1", "Caméra 1", "防犯カメラ1", "Fotocamera1"},
		{"Camera2", "摄像机2", "Kamera 2", "מצלמה 2", "Kamera 2", "Câmera2", "Cámara2", "Caméra 2", "防犯カメラ2", "Fotocamera2"},
		{"Open", "打开", "Offen", "לפתוח", "Otwórz", "Abrir", "Abierto", "Ouvrir", "開く", "Aprire"},
		{"delete", "删除", "Löschen", "למחוק", "Usuń", "Eliminar", "Eliminar", "Effacer", "削除", "Eliminare"},
		{"Delete All", "删除全部", "Alles löschen", "מחק הכל", "Usuń wszystko", "Eliminar tudo", "Eliminar todos", "Supprimer tout", "すべて削除", "Eliminare tutto"},
		{"closure", "关闭", "Schließung", "חזור", "Zamknij", "fecho", "cierre", "Fermer", "閉じる", "chiusura"},
		{"File Corrupted", "文件损坏", "Beschädigung der datei.", "השחתת קבצים", "Uszkodzenie pliku", "O arquivo está corrompido", "Archivo dañado", "Corruption du fichier", "ファイルの破損", "Poškození souboru"},
		{"Video", "视频", "Video", "וידאו", "Wideo", "Vídeo", "Video", "Vidéo", "ビデオ", "Video"},
		{"System", "系统", "System", "מערכת", "System", "Sistema", "Sistema", "Système", "システム設定", "Sistema"},
		{"Door", "门口机", "Tür", "דלת", "Drzwi", "Porta", "Puerta", "Platine de rue", "玄関ドアフォン", "Porta"},
		{"Camera", "摄像机设置", "Kamera", "מצלמה", "Kamera", "Câmera", "Cámara", "Caméra", "防犯カメラ設定", "Telecamera"},
		{"Network", "网络", "Netzwerk", "רשת", "Sieć", "Rede", "Lared", "Réseau", "ネットワーク", "Rete"},
		{"Scene", "情景", "Diashow", "תרחיש", "Ramka cyfrowa", "Cena", "Escena", "Multimedia", "待ち受け", "Scena"},
		{"Advanced", "高级设置", "Werkseinstellung", "הגדרות מתקדמות", "Zaawansowane", "Avançado", "Avanzado", "Avancé", "詳細設定", "Avanzate"},
		{"System info", "系统信息", "ysteminfo", "מידע מערכת", "Informacje", "Informação sistema", "Información sistema", "Infos système", "システム情報", "Info. sistema"},
		{"APP online preview", "APP在线预览", "APP Online-Vorschau", "תצוגת אפליקציה", "APP podgląd online", "APP Antevisão on-line", "Vista previa en línea de la APP", "Visualisation en cours depuis Smartphone", "アプリオンライン視聴中", "Anteprima online dell'APP"},
		{"REC", "录像", "REC", "מקליט", "REC", "Gravação", "Grabación", "Vidéo", "ビデオ録画", "Registrazione"},
		{"Snapshot", "拍摄中", "Schnappschuss", "צילום תמונה", "Zdjęcie", "Foto", "Foto", "Instantané", "撮影中", "Istantaneo"},
		{"DOOR1 CALLING", "门口机1呼叫", "TÜRE1 RUFEN", "דלת 1 מתקשרת", "Dzwonią Drzwi 1", "Porta1 chamando", "Puerta1 llamando", "Appel depuis platine 1", "玄関ドアフォン1呼び出し中", "Porta1 Chiamata"},
		{"DOOR2 CALLING", "门口机2呼叫", "TÜRE2 RUFEN", "דלת 2 מתקשרת", "Dzwonią Drzwi 2", "Porta2 chamando", "Puerta2 llamadas", "Appel depuis platine 2", "玄関ドアフォン2呼び出し中", "Porta2 Chiamata"},
		{"Device is busy", "设备繁忙", "Gerät ist beschäftigt", "המכשיר תפוס", "Urządzenie jest zajęte", "Dispositivo está ocupado", "El dispositivo está ocupado", "Le périphérique est occupé", "設備が忙しい", "Il dispositivo è occupato"},
		{"Insufficient card capacity", "SD卡内存不足", "Unzureichende Kartenkapazität", "אין מספיק זיכרון בכרטיס SD", "Mała pamięć karty SD", "Capacidade insuficiente do cartão", "Capacidad de tarjeta insuficiente", "Espace carte insuffisant", "SDカードの容量不足", "Capacità scheda insufficiente"},
		{"Movie", "电影", "Videos", "סרט", "Filmy", "Filme", "Película", "Film", "動画", "Film"},
		{"Music", "音乐", "Musik", "מוסיקה", "Muzyka", "Música", "Música", "Musique", "音楽", "Musica"},
		{"Photo", "照片", "Foto", "תמונה", "Zdjęcia", "Foto", "Foto", "Photo", "写真", "Foto"},
		{"Files", "文件", "Daten", "קבצים", "Plik", "Arquivos", "Archivos", "Fichiers", "ファイル", "File"},
		{"Call record", "呼叫记录", "Anrufaufzeichnung", "הקלטת שיחות", "Rejestr połączeń", "Registro de chamada", "Registro de llamadas", "Journal d'appels", "通話記録", "Registro chiamate"},
		{"Message record", "留言记录", "Nachricht Information", "הקלטת הודעות", "Lista wiadomości", "Registro mensagens", "Registro mensajes", "Messagerie", "伝言メモ", "Registro messaggi"},
		{"Motion detection", "移动侦测", "Bewegungserkennung", "גלאי תנועה", "Rejestr detekcji", "Detector de movimento", "Detección de movimiento", "Détections", "動体検出記録", "Movimenti rilevati"},
		{"Alarm information", "报警日志", "Alarminformationen", "מידע על אזעקה", "Rejestr alarmów", "Informações sobre alarme", "Información de alarma", "Alertes", "警報情報", "Informazioni sugli allarmi"},
		{"Device ID", "设备号", "Geräte-ID", "מזהה מכשיר", "Adres monitora", "ID de dispositivo", "Identificación del dispositivo", "Identifiant moniteur", "機器ID", "ID del dispositivo"},
		{"Time", "时间", "Uhrzeit", "זמן ", "Czas", "Hora", "Hora ", "Date / Heure", "時間", "Tempo"},
		{"Date format", "日期格式", "Datumsformat", "פורמט תאריך", "Format daty", "Formato de data", "Formato de fecha", "Format date", "日付書式", "Formato data"},
		{"Clock", "时钟", "Uhr", "שעון", "Zegar", "Relógio", "Reloj", "Horloge", "時計", "Orologio"},
		{"Language", "语言", "Sprache", "שפה", "Język", "Idioma", "Idioma", "Langue", "言語", "Lingua"},
		{"Keytone", "按键音", "Tastenton", "עוצמת מקשים", "Dźwięki operacji", "Tom de toque", "Tono de teclado", "Son tactile", "操作音", "Tono dei tasti"},
		{"Mobile detection preview", "移动侦测预览", "MD-Vorschau", "תצוגה מקדימה באפליקציה", "Podgląd detekcji ruchu", "Visualização do MD", "Vista previa de MD", "Aperçu D.M", "動体検出即時視聴", "Anteprima MD"},
		{"Ringback", "回铃声", "Ton Türstation", "חיוג חוזר", "Sygnał łączenia", "Chamar novamente", "Volver a llamar", "Tonalité platine", "玄関ドア側呼出音", "Richiamare"},
		{"Gate2 unlock time", "Gate2解锁时间", "Türöffnerzeit Monitor", "ביטול נעילה שער 2", "Brama 2 czas otwarcia", "Porta2 tempo desbloqueio", "Puerta2 Tiempo desbloqueo", "Temporisation relais int.", "門扉2解錠時間", "Tempo di sblocco Porta2"},
		{"Unlocking voice", "开锁提示音", "Schiess einen ton ab", "צליל ביטול נעילה", "Odblokuj dźwięk", "Desbloqueio prompt tom", "Sonido de alerta de bloqueo", "Tonalité de déclenchement de verrouillage", "ロック解除トーン", "Smontatura del prompt"},
		{"Admin setting", "管理员设置", "Admin-Einstellung", "הגדרות מנהל", "Ustawienia Admin.", "Configuração administrador", "Ajuste de administración", "Paramètres admin.", "管理者設定", "Imposta amministratore"},
		{"YY/MM/DD", "年/月/日", "JJ/MM/TT", "YY/MM/DD", "RR-MM-DD", "Yy/mm/dd", "Yy/mm/dd", "AA/MM/JJ", "年/月/日", "AA/MM/GG"},
		{"MM/DD/YY", "月/日/年", "MM/TT/JJ", "MM/DD/YY", "MM-DD-RR", "Mm/dd/yy", "Mm/dd/yy", "MM/JJ/AA", "月/日/年", "MM/GG/AA"},
		{"DD/MM/YY", "日/月/年", "TT/MM/JJ", "DD/MM/YY", "DD-MM-RR", "Dd/mm/yy", "Dd/mm/yy", "JJ/MM/AA", "日/月/年", "GG/MM/AA"},
		{"On", "开", "Ein", "הפעל", "Wł.", "On", "On", "On", "オン", "On"},
		{"Off", "关", "Aus", "כבוי", "Wył.", "Off", "Off", "Off", "オフ", "Off"},
		{"S", "S", "S", "שניות", "S", "S", "S", "S", "秒", "S"},
		{"Non ID1 ,cannot be set", "非ID1不可设置", "Nicht ID1,Nicht einstellbar", "כתובת 1 לא ניתנת להגדרה", "Nie można ustawić ID1", "Não é possível definir o ID1", "No se puede configurar sin id1", "Non - id1 non configurable", "ID 1以外は設定できません", "Non ID1 inoperabile"},
		{"Dormant, non réglable", "休眠,不可设置", "Ruhend,Nicht einstellbar", "במצב שינה לא ניתן לתכנות ", "Noc,Nieustawialny", "Dormindo,Não configurável", "Dormir, não pode ser definido", "Hibernation, non réglable", "スリープ、設定不可", "Dormiente,Non impostabile"},
		{"room number", "房号", "Klingeltaste", "מספר חדר", "Numer pokoju", "número habitação", "número de habitación", "Numéro de pièce", "部屋番号", "numero di Camera"},
		{"Router address", "路由器地址", "Router-Adresse", "כתובת הנתב", "Adres IP", "Endereço router", "Dirección del enrutador", "Adresse routeur", "IPアドレス", "Indirizzo del router"},
		{"Change password", "修改密码", "Passwort ändern", "שנה סיסמה", "Zmień hasło", "Mudar senha", "Cambia la contraseña", "Changer le code accès", "パスワードの変更", "Cambiare la password"},
		{"Admin reset", "恢复管理员", "Admin-Reset", "איפוס מנהל", "Reset administratora", "Reset administrador", "Reset administrador", "Réinitialiser administrateur", "管理者設定リセット", "Ripristino amministratore"},
		{"Admin", "管理员", "Admin", "מנהל", "Admin", "Admin", "Administración", "Administrateur", "管理者", "Amministratore"},
		{"Admin reset？", "恢复管理员？", "Admin-Reset？", "איפוס מנהל?", "Reset administratora？", "Reset administrador？", "Reset administrador？", "Réinitialiser administrateur？", "管理者設定をリセットしますか？", "Ripristino amministratore？"},
		{"OK", "确认", "OK", "בסדר", "OK", "OK", "OK", "OK", "OK", "OK"},
		{"Cancel", "取消", "Abbrechen", "לבטל", "Anulować", "Cancelar", "Cancelar", "Annuler", "キャンセル", "Cancellare"},
		{"Status", "状态", "Status", "סטטוס", "Status", "Estado", "Estado", "État", "稼働状況", "Stato"},
		{"Lock unlock time", "Lock解锁时间", "Entsperrzeit Türöffner", "זמן פתיחת המנעול", "Zamek czas otwarcia", "Tempo desbloqueio porta", "Tiempo desbloqueo puerta", "Temporisation serrure", "錠解錠時間", "Blocca il tempo di sblocco"},
		{"Gate1 unlock time", "Gate1解锁时间", "Türöffnerzeit Monitor", "זמן פתיחת שער 1", "Brama 1 czas otwarcia", "Porta1 tempo desbloqueio", "Puerta1 Tiempo desbloqueo", "Temporisation portail ", "門扉1解錠時間", "Tempo di sblocco Porta1"},
		{"Record mode", "记录模式", "Aufnahme-Modus", "מצב הקלטה", "Tryb nagrywania", "Modo gravação", "Modo grabación", "Mode enregistrement", "記録モード", "Modalità di registrazione"},
		{"Motion detection", "移动侦测", "Bewegungserkennung", "גלאי תנועה", "Detekcja ruchu", "Detector de movimento", "Motion detection ", "Détection mouvement", "動体検出感度", "Rilevamento del movimento"},
		{"NULL", "MD灵敏度", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "sensibilità"},
		{"Movement detection record", "移动侦测记录", "MD-Aufzeichnungen", "הקלטות זיהוי תנועה", "Zapis z detekcji", "Gravação MD", "Registros MD", "Journal D.M", "動体検出記録方法", "Registro MD"},
		{"Motion detection duration", "移动侦测时长", "Erkennungszeit", "משך זיהוי תנועה", "Czas detekcji ruchu", "Duração deteção movimento", "Movement detection duration", "Durée D.M", "動体検出記録時間", "Durata cattura movimento"},
		{"Message", "留言", "Nachricht hinterlassen", "הודעה", "Zostaw wiadomość", "Mensagem", "Mensaje", "Message", "不在伝言機能", "Messaggio"},
		{"Message time", "留言时长", "Nachrichtendauer", "זמן הודעה", "Długość wiadomości", "Hora da mensagem", "Tiempo de mensaje", "Durée message", "伝言時間", "Ora del messaggio"},
		{"Ring setting", "铃声设置", "Klingeltoneinstellung", "הגדרת צלצול", "Ustawienia dzwonka", "Configuração toque", "Ajuste del timbre", "Réglage sonnerie", "呼び鈴設定", "Regolazione dell'anello"},
		{"Please close the scene", "请关闭情景设置", "Bitte schließen Sie die Szene", "אנא סגור את התרחיש", "Proszę zamknąć scenę", "Por favor, feche cenário", "Por favor cierre la escena", "Fermer lecteur multimédia", "待ち受け機能をオフにしてください", "Chiudi la scena"},
		{"In away mode", "离家模式中", "Im Abwesenheitsmodus", "לא בבית", "W trybie wyjazdowym", "Modo fora", "En modo fuera", "En mode absent", "外出モード中は不在伝言機能をオフにできません", "In modalità fuori casa"},
		{"Low", "低", "Niedrig", "נמוך", "Niska", "Baixo", "Bajo", "Basse", "低い", "Basso"},
		{"middle", "中", "Mitte", "רגיל", "Średnia", "meio", "medio", "Moyenne", "中", "mezzo"},
		{"high", "高", "hoch", "גבוה", "Wysoka", "Alto", "alto", "Haute", "高い", "alto"},
		{"Snapshot", "拍摄", "Schnappschuss", "צילום תמונה", "Zdjęcie", "Foto", "Foto", "Photo", "写真撮影", "Istantaneo"},
		{"Camera model", "摄像机型号", "Kamera modell", "דגם מצלמה", "Model aparatu", "Modelo de câmera", "Modelo de cámara", "Modèle caméra", "防犯カメラ機種", "Modello di fotocamera"},
		{"Dahua", "大华", "Dahua", "Dahua", "Dahua", "Dahua", "Dahua", "Dahua", "Dahua社製", "Dahua"},
		{"Hikvision", "海康", "Hikvision", "Hikvision", "Hikvision", "Hikvision", "Caminata", "Hikvision", "Hikvision社製", "Hikvision"},
		{"Camera IP address", "摄像机IP地址", "IP Adresse der Kamera", "כתובת IP של המצלמה", "Adres IP kamery", "Endereço IP da câmera", "Dirección IP de la cámara", "Adresse IP caméra", "防犯カメラIPアドレス", "Indirizzo IP della telecamera"},
		{"Account number", "账号", "Login", "מספר חשבון", "Numer konta", "Número da conta", "Número de cuenta", "Identifiant compte", "アカウント名", "Numero di conto"},
		{"Password", "密码", "Passwort", "סיסמה", "Hasło", "Senha", "Contraseña", "Mot de passe", "パスワード", "Password"},
		{"Ring1", "铃声1", "Klingelton 1", "צלצול 1", "Dzwonek 1", "Toque1", "Timbre1", "Sonnerie 1", "呼び鈴1", "Suoneria1"},
		{"Ring2", "铃声2", "Klingelton 2", "צלצול 2", "Dzwonek 2", "Toque2", "Timbre2", "Sonnerie 2", "呼び鈴2", "Suoneria2"},
		{"Ring3", "铃声3", "Klingelton 3", "צלצול 3", "Dzwonek 3", "Toque3", "Timbre3", "Sonnerie 3", "呼び鈴3", "Suoneria3"},
		{"Ring1/Door1", "铃声1/门1", "Ring1/Tür1", "צלצול 1/דלת 1", "Pierścień1/Drzwi1", "Toque1/porta1", "Timbre1/puerta1", "Sonnerie1 / Platine1", "呼び鈴1/玄関ドア1", "Suoneria1/Porta1"},
		{"Ring2/Door1", "铃声2/门1", "Ring2/Tür1", "צלצול 2/דלת 1", "Pierścień2/Drzwi1", "Toque2/porta1", "Timbre2/puerta1", "Sonnerie2 / Platine1", "呼び鈴2/玄関ドア1", "Suoneria2/Porta1"},
		{"Ring3/Door1", "铃声3/门1", "Ring3/Tür1", "צלצול 3/דלת 1", "Pierścień3/Drzwi1", "Toque3/porta1", "Timbre3/puerta1", "Sonnerie3 / Platine1", "呼び鈴3/玄関ドア1", "Suoneria3/Porta1"},
		{"Ring1/Door2", "铃声1/门2", "Ring1/Tür2", "צלצול 1/דלת2", "Pierścień1/Drzwi2", "Toque1/porta2", "Timbre1/puerta2", "Sonnerie1 / Platine2", "呼び鈴1/玄関ドア2", "Suoneria1/Porta2"},
		{"Ring2/Door2", "铃声2/门2", "Ring2/Tür2", "צלצול 2/דלת2", "Pierścień2/Drzwi2", "Toque2/porta2", "Timbre2/puerta2", "Sonnerie2 / Platine2", "呼び鈴2/玄関ドア2", "Suoneria2/Porta2"},
		{"Ring3/Door2", "铃声3/门2", "Ring3/Tür2", "צלצול 3/דלת 2", "Pierścień3/Drzwi2", "Toque3/porta2", "Timbre3/puerta2", "Sonnerie3 / Platine2", "呼び鈴3/玄関ドア2", "Suoneria3/Porta2"},
		{"Schedule", "时间表", "Zeiteinstellung", "לוח זמנים", "Harmonogram", "Calendário", "Calendario", "Tranche horaire", "スケジュール", "Programma"},
		{"Ring time", "响铃时长", "Zeiteinstellung", "זמן צלצול", "Czas trwania melodii", "Tempo de toque", "Tiempo de timbre", "Durée sonnerie", "呼び鈴鳴動時間", "Tempo di squillo"},
		{"Ring mode", "铃声模式", "Art des Klingeltons", "מצב צלצול", "Rodzaj dzwonka", "Modo de toque", "Modo del timbre", "Mode sonnerie", "音色読み出し場所", "Modalità suoneria"},
		{"Ring select", "铃声选择", "Auswahl Klingelton", "בחירת צלצול", "Wybór dzwonka", "Seleção toque", "Anillo seleccionar", "Sonnerie", "呼び鈴音色選択", "Seleziona anello"},
		{"Ring volume", "铃声音量", "Lautstärke Klingelton", "עוצמת הצלצול", "Głośność dzwonka", "Volume toque", "Volumen timbre", "Volume", "呼び鈴音量", "Volume suoneria"},
		{"Standard", "标准", "Standard", "רגיל", "Standardowy", "Padrão", "Estándar", "Classique", "内蔵メモリー", "Standard"},
		{"Customized", "自定义", "Angepasst", "מותאם אישית", "Użytkownika", "Personalizado", "Personalizado", "Personnalisée", "SDカード", "Personalizzato"},
		{"Net pairing Mode", "配网模式", "Net-Pairing-Modus", "סוג חיבור רשת", "Tryb parowania sieci", "Modo emparelhamento rede", "Modo emparejamiento red", "Mode appairage", "Tuyaペアリング方法", "Modalità collegamento rete"},
		{"EZ Mode", "WIFI 快连", "EZ-Modus", " מצב EZ", "Tryb EZ", "Modo EZ", "Modo EZ", "Mode EZ", "かんたんモード", "EZ mode"},
		{"Cable", "有线配网", "Lan", "מצב כבל", "Kabel", "Cabo", "Cable", "Câble", "有線LAN", "LAN cable"},
		{"Statically IP", "静态分配", "Statisch IP", "הוקע באופן סטטי", "Statyczne IP", "alocados estaticamente", "Asignación estática", "Distribution statique", "せいてきわりつけ", "allocazione statica"},
		{"Dynamic IP", "动态分配", "Dynamische IP", "התפקיד דינמי", "Dynamiczne IP", "alocação dinâmica", "Asignación dinámica", "Distribution dynamique", "動的割当て＃ドウテキワリアテ＃", "allocazione dinamica"},
		{"WLAN", "WLAN", "WLAN", "WLAN", "WLAN", "WLAN", "WLAN", "WLAN", "Wi-Fi設定", "WLAN"},
		{"Network information", "网络信息", "Netzwerkinformationen", "מידע רשת", "Informacje o sieci", "Informações de rede", "Información de la red", "Infos réseau", "ネットワーク情報", "Informazioni di rete"},
		{"QR code", "二维码配网", "QR-Code", "קוד QR", "Kod QR", "Código QR", "Código QR", "QR Code", "QRコード", "QR Code"},
		{"Add manually", "手动添加", "Manuell hinzufügen", "להוסיף באופן ידני", "Dodaj ręcznie", "Adicionar manualmente", "Agregar manualmente", "Ajouter manuellement", "手動追加", "Aggiungi manualmente"},
		{"Disconnect WiFi", "是否断开WiFi", "WiFi trennen", "נתק WiFi", "Zapomnij sieć", "Desconecte o WiFi", "Desconectar wifi", "Déconnecté WIFI", "Wi-Fiを切断しますか？", "Disconnettere Wi-Fi"},
		{"IP address", "IP地址", "IP Adresse", "כתובת IP", "Adres IP", "endereço de IP", "dirección IP", "Adresse IP", "IPアドレス", "indirizzo IP"},
		{"MAC", "MAC", "MAC", "כתובת MAC", "Adres MAC", "MAC", "MAC", "MAC", "MACアドレス", "MAC"},
		{"WiFi account", "WiFi账号", "WLAN-Konto", "חשבון WiFi", "Konto Wi-Fi", "Conta WiFi", "Cuenta WiFi", "Réseau WIFI", "Wi-Fiネットワーク名", "Conto Wi-Fi"},
		{"Password", "密码", "Passwort", "סיסמה", "Hasło", "Senha", "Clave", "Mot de passe", "パスワード", "Password"},
		{"Digital photo", "数码相框", "Digitaler Bilderrahmen", "תמונה דיגיטלית", "Ramka cyfrowa", "Foto digital", "Foto digital", "Cadre photo numérique", "デジタルフォトフレーム", "Foto digitale"},
		{"Digital photo switch time", "相片切换时间", "Fototour Zeit", "זמן החלפת תמונה דיגיטלית", "Czas sekwencji", "Tempo mudança fotos", "Tiempo de cambio de foto", "Durée affichage photo", "写真切り替え時間", "Tempo foto successiva"},
		{"Background music", "背景音乐", "Hintergrundmusik", "מוזיקת ​​רקע", "Muzyka w tle", "Música de fundo", "Música de fondo", "Musique de fond", "BGM", "Musica di sottofondo"},
		{"Background music volume", "背景音量", "Hintergrundlautstärke", "עוצמת הקול של מוזיקת רקע", "Głośność muzyki w tle", "Volume música fundo", "Volumen música fondo", "Volume musique fond", "BGMボリューム", "Volume musica sottofondo"},
		{"Please close MD", "请关闭MD", "Bitte MD schließen", "כיבוי זיהוי תנועה", "Proszę zamknąć MD", "Por favor, feche MD", "Por favor cierre MD", "Veuillez fermer D.M", "動体検出機能をオフにしてください", "Si prega di chiudere MD"},
		{"Format SD", "格式化SD卡", "SD Karte formatieren", "פרמט כרטיס SD", "Formatuj kartę SD", "Formatar sd", "Formato SD", "Formater SD", "SDカードのフォーマット", "Formatta SD"},
		{"Indoor monitor reset", "室内机复位", "Innenstation Reset", "איפוס מסך פנימי", "Ustawienia fabryczne", "Reset do monitor interno", "Reset monitor interno", "Réinitialiser moniteur", "室内モニターリセット", "Ripristino dell'unità interna"},
		{"Door station reset", "门口机复位", "Türstation Reset", "איפוס הפנל החיצוני", "Reset stacji zewn", "Reset unidade exterior", "Reset estación puerta", "Réinitialiser platine", "玄関ドアフォンリセット", "Ripristino posto esterno"},
		{"Door station upgrade", "门口机升级", "Türstations-Upgrade", "עדכון הפנל החיצוני", "Aktualizacja Stacji", "Atualização unidade exterior", "Actualización unidad exterior", "Mise à jour platine", "玄関電話のアップグレード", "Türstations-Upgrade"},
		{"Restart system", "重启系统", "Neustart des Systems", "הפעלה מחדש של המערכת", "Uruchom ponownie system", "reinicialização do sistema", "reinicio del sistema", "Redémarrer", "システムの再起動", "Riavvia il sistema"},
		{"APP unlock", "APP开锁", "APP entsperren", "פתיחה באפליקציה", "app_unlock", "APP Desbloqueio", "APP Desbloquear", "Ouverture par APP", "アプリ解錠対象門扉", "Sblocco APP"},
		{"Standby mode", "待机模式", "Standby Modus", "מצב המתנה", "Tryb czuwania", "Modo de espera", "Modo de espera", "Mode veille", "スタンバイモード", "Modalità standby"},
		{"Format SD card?", "格式化SD卡？", "SD-Karte formatieren?", "מחיקת קבצים בכרטיס SD?", "Formatuj kartę SD？", "Formatar cartão SD?", "Formatear tarjeta SD?", "Formater carte SD?", "SDカードをフォーマットしますか?", "Formattare la scheda SD?"},
		{"Formatting", "格式化中", "Formatierung", "מחיקה", "Formatowanie", "Formatando", "Formato", "Formatage", "書式設定", "Formattazione"},
		{"SD card is being scanned", "正在扫描SD卡", "Sie scannt die sd card", "כרטיס SD נסרק", "Karta SD jest skanowana", "Digitalizando o cartão sd", "Escaneando la tarjeta sd", "Scanner la carte sd en cours", "SDカードをスキャンしています", "Scansione delle carte sd"},
		{"Successful format", "格式化成功", "Erfolgsformat", "המחיקה הצליחה", "Udany format", "Formatação bem sucedida", "Formato exitoso", "Formatage réussi", "フォーマットに成功しました", "Formato riuscito"},
		{"Indoor monitor reset?", "室内机复位？", "Indoor monitor Reset?", "איפוס מסך פנימי?", "Ustawienia fabryczne?", "Reset do monitor interno?", "Reset monitor interno?", "Réinitialiser le moniteur?", "室内モニターをリセットしますか?", "Ripristino dell'unità interna？"},
		{"Door station reset？", "门口机复位?", "Türstation zurücksetzen?", "לאפס את הפנל החיצוני?", "Reset stacji zewn？", "Reset unidade exterior ?", "Reset estación puerta？", "Réinitialiser platine？", "玄関ドアフォンをリセットしますか?", "Ripristino dell'unità esterna？"},
		{"Restoring the system...", "恢复系统中...", "System wiederherstellen...", "משחזר את המערכת...", "Przywracam system...", "Restaurando o sistema ...", "Restaurar el sistema ...", "Restauration du système ...", "システムを復元しています...", "Ripristino del sistema..."},
		{"Door station upgrade？", "是否更新", "Türstations-Upgrade?", "שדרוג הפנל החיצוני?", "Aktualizacja Stacji?", "Atualização da unidade exterior？", "Actualización de la unidad exterior？", "Mettre à jour la platine de rue？", "アップグレードしますか?", "Aggiornamento dell'unità esterna？"},
		{"Check outdoor status", "检测户外机状态", "Prüfen draussen status", "בדוק את מצב הפנל החיצוני", "Sprawdź stan na zewnątrz", "Verifique o status da porta", "Verifique el estado de la puerta", "Statut platine extérieur", "玄関ドアフォンの状態を確認...", "Controlla lo stato esterno"},
		{"device offline", "设备离线", "Gerät offline", "המכשיר במצב לא מקוון", "brak urządzenia", "dispositivo offline", "dispositivo fuera de línea", "Appareil hors ligne", "設備がオフラインになっている", "dispositivo offline"},
		{"no upgrade FW", "没有升级包", "kein FW-Upgrade", "אין שדרוג FW", "brak aktualizacji FW", "Sem atualização FW", "Sin actualización FW", "Aucune mise à jour", "アップグレードパッケージなし", "nessun aggiornamento FW"},
		{"upgrading", "正在升级", "Upgrade durchführen", "שדרוג", "modernizacja", "atualizando", "actualización", "Mise à jour", "実行中...(電源を切らないでください)", "aggiornamento"},
		{"update successed", "升级成功", "Aktualisierung erfolgreich", "העדכון הצליח", "aktualizacja powiodła się", "Atualização com sucesso", "Actualización exitada", "Mise à jour réussie", "アップグレードに成功しました", "aggiornamento riuscito"},
		{"Restart system? ", "是否重启", "ob neu starten", " להפעיל מחדש את המערכת ?", "Uruchom ponownie system?", "reinicialização do sistema?", "reinicio del sistema?", "Redémarrer le système?", "システムを再起動しますか?", "Riavviare il sistema?"},
		{"Hardware version", "硬件版本", "Hardware Version", "גרסת החומרה", "Wersja sprzętowa", "Versão de hardware", "Versión del hardware", "Version matériel", "ハードウェアバージョン", "Versione hardware"},
		{"Software version", "软件版本", "Software Version", "גרסת תוכנה", "Wersja oprogramowania", "Versão do software", "Versión del software", "Version firmware", "ソフトウェアバージョン", "Versione software"},
		{"Firmware version", "固件版本", "Firmware Version", "גרסת קושחה", "Firmware", "Versão do firmware", "Versión de firmware", "Version logiciel", "ファームウェアバージョン", "Versione del firmware"},
		{"Door 1 version", "门口机1版本", "Tür 1 Version", "גירסת דלת 1", "Wersja drzwi 1", "Versão Porta 1 ", "Versión de la puerta 1", "Version platine 1", "玄関ドアフォン1バージョン", "Versione porta 1"},
		{"Door 2 version", "门口机2版本", "Tür 2 Version", "גרסת דלת 2", "Wersja drzwi 2", "Versão Porta 2", "Versión de la puerta 2", "Version platine 2", "玄関ドアフォン2バージョン", "Versione porta 2"},
		{"Door 1 version", "门口机1发布日期", "Tür 1 Version", "גירסת דלת 1", "Wersja drzwi 1", "Versão Porta 1 ", "Versión de la puerta 1", "Version platine 1", "玄関ドアフォン1バージョン", "Versione porta 1"},
		{"Door 2 version", "门口机2发布日期", "Tür 2 Version", "גרסת דלת 2", "Wersja drzwi 2", "Versão Porta 2", "Versión de la puerta 2", "Version platine 2", "玄関ドアフォン2バージョン", "Versione porta 2"},
		{"Release date", "发布日期", "Datum aktivieren", "תאריך הוצאה", "Data wydania", "Data de lançamento", "Fecha de lanzamiento", "Evoya date ", "ソフトウェア公開日", "Data di rilascio"},
		{"SD size", "SD卡容量", "Kapazität der SD-Karte", "גודל SD", "Pojemność karty SD", "Tamanho do SD", "Tamaño SD", "Capacité SD", "SDカード容量", "Dimensione SD"},
		{"Tuya ID", "涂鸦ID", "Tuya ID", "Tuya ID", "Tuya ID", "Tuya ID", "Tuya ID", "Tuya ID", "Tuya ID", "Tuya ID"},
		{"Get Tuya ID", "涂鸦ID读取", "Tuya ID holen ", "קבל מזהה מ Smartvill", "Odczyt Tuya ID", "Obtenha tuya id", "Obtener Tuya ID", "Identifiant Tuya", "Tuya IDの取得", "Ottieni l'ID Tuya"},
		{"Replace ?", "是否替换", "Ersetzen ?", " החלף ?", "Zastępować ?", "Substituir ?", "Reemplazar ?", "Remplacer ?", "現在のIDと置き換えますか?", "Sostituire ?"},
		{"No Tuya files", "无涂鸦文件", "Keine Tuya-Dateien", "אין קבצים באפליקצית Smartvil", "Brak plików Tuya", "Sem ficheiros Tuya", "No hay archivos tuya", "Aucun fichier Tuya", "Tuyaファイルなし ", "Nessun file Tuya"},
		{"No valid UUID", "无有效UUID", "Keine gültige UUID", "UUID לא בשימוש לא נמצא", "Brak prawidłowego UUID", "Nenhum UUID válido", "No hay UUID válido", "UUID invalide", "有効なUUIDがありません", "Nessun UUID valido"},
		{"file error", "文件错误", "Dateifehler", " שגיאת קובץ", "błąd pliku", "erro de arquivo", "error de archivo", "Erreur fichier", "ファイルエラー", "errore di file"},
		{"File read and write error", "文件读写错误", "Fehler beim Lesen der Datei", "שגיאה בקריאת וכתיבת הקובץ", "Błąd odczytu i zapisu pliku", "Erro de leitura e gravação do arquivo", "Error de lectura y escritura de archivo", "Erreur lecture /  écriture fichier", "ファイル読み書きエラー", "Errore di lettura e scrittura del file"},
		{"Password input", "密码输入", "Eingabe Passwort", "הזנת סיסמה", "Wprowadzanie hasła", "Introduza senha", "Introducir contraseña", "Saisir code accès", "パスワード入力", "Inserimento password"},
		{"Please enter", "请输入", "Bitte bestätigen", "בבקשה היכנס", "Podaj", "Por favor, insira", "Por favor escribe", "Veuillez saisir", "入力してください", "Prego entra"},
		{"Please input a password", "请输入密码", "Bitte Passwort eingeben", "נא להזין סיסמה", "Wprowadź hasło", "Insira uma senha", "Ingrese una contraseña", "Veuillez saisir code accès", "パスワードを入力してください", "Inserisci una password"},
		{"Please enter a new password", "请输入新密码", "Bitte neues Passwort eingeben", "נא להזין סיסמה חדשה", "Wprowadź nowe hasło", "Por favor, insira uma nova senha", "Ingrese una nueva contraseña", "Saisir nouveau code accès", "新しいパスワードを入力してください", "Inserisci una nuova password"},
		{"Please enter the IP address of the camera", "请输入摄像机IP地址", "Bitte geben Sie die IP-Adresse der Kamera ein", "הזן את כתובת ה-IP של המצלמה", "Podaj adres IP kamery", "Por favor, insira o endereço IP da câmera", "Ingrese la dirección IP de la cámara", "Veuillez saisir l'adresse IP de la caméra", "防犯カメラのIPアドレスを入力してください", "Inserisci l'indirizzo IP della telecamera"},
		{"Please enter camera password", "请输入摄像机密码", "Bitte Passwort der Kamera eingeben", "נא להזין את סיסמת המצלמה", "Wprowadź hasło do aparatu", "Por favor, digite a senha da câmera", "Ingrese la contraseña de la cámara", "Votre mot de passe de la caméra", "防犯カメラのパスワードを入力してください", "Inserisci la password della fotocamera"},
		{"Please enter camera account", "请输入摄像机账号", "Bitte Kamera-Konto eingeben", "נא להזין חשבון מצלמה", "Wprowadź konto kamery", "Por favor, insira a conta da câmera", "Ingrese la cuenta de la cámara", "Votre identifiant de la caméra", "防犯カメラのアカウント名を入力してください", "Inserisci l'account della fotocamera"},
		{"Please add a Lock card", "请添加Lock卡片", "Bitte fügen Sie die Sperrkarte hinzu", "אנא הוסף את כרטיס הנעילה", "Dodaj kartę blokady", "Adicione um cartão de bloqueio", "Agregue una tarjeta de bloqueo", "Ajouter carte accès serrure", "NULL", "Si prega di aggiungere una carta di blocco"},
		{"Please add the Gate1 card", "请添加Gate1卡片", "Bitte fügen Sie eine Gate1-Karte hinzu", "אנא הוסיפו את כרטיס השער1", "Dodaj kartę Gate1", "Adicione o cartão Portão1", "Agregue la tarjeta GATE1", "Ajouter carte accès portail 1", "NULL", "Aggiungi la scheda Gate1"},
		{"Please delete the Lock card", "请删除Lock卡片", "Bitte löschen Sie die Sperrkarte", "אנא מחק את כרטיס הנעילה", "Usuń kartę blokady", "Eliminar cartão de Portão", "Elimina la tarjeta de bloqueo", "Supprimer carte accès serrure", "NULL", "Si prega di eliminare la carta di blocco"},
		{"Please delete the Gate1 card", "请删除Gate1卡片", "Bitte löschen Sie die Gate1-Karte", "אנא מחק את כרטיס שער 1", "Proszę usunąć kartę Gate1", "Eliminar o cartão Portão1", "Elimine la tarjeta GATE1", "Supprimer carte accès portail 1", "NULL", "Si prega di eliminare la scheda Gate1"},
		{"Connecting, please wait...", "正在连接，请稍等...", "verbinden.，Festhalten...", "מתחבר, אנא המתן...", "Złączony,Proszę czekać", "Conectando, por favor, espere ...", "Conectando, espere por favor...", "Connexion, veuillez patienter ...", "接続中です、少々お待ちください...", "Connessione in corso, attendere..."},
		{"successful connection", "连接成功", "erfolgreiche Verbindung", "מחובר בהצלחה", "pomyślnie połączony", "conexão bem sucedida", "conexión exitosa", "Connexion réussie", "接続成功", "connessione andata a buon fine"},
		{"Connection failed", "连接失败", "Verbindung fehlgeschlagen", "חיבור נכשל", "Połączenie nieudane", "A conexão falhou", "La conexión falló", "Echec de connexion", "接続に失敗しました", "Connessione fallita"},
		{"Wifi account is empty", "Wifi账号为空", "WLAN-Konto ist leer", "חשבון Wifi ריק", "Konto Wi-Fi jest puste", "A conta wifi está vazia", "La cuenta WiFi está vacía", "Compte WiFi vide", "Wi-Fiネットワーク名が空です", "L'account Wi-Fi è vuoto"},
		{"Wifi password is too short", "Wifi密码太短", "Wifi Passwort zu kurz", "סיסמת WIFI קצרה מידי", "WiFi - hasło za krótkie", "A senha wifi é muito curta", "La contraseña wifi es demasiado corta", "Code accès trop court", "パスワードが短すぎます", "La password Wi-Fi è troppo breve"},
		{"not found", "没有找到", "nicht gefunden", "לא נמצא", "nie znaleziono", "não encontrado", "extraviado", "pas trouvé", "見つかりませんでした", "non trovato"},
		{"password is too short", "密码太短", "das Passwort ist zu kurz", "סיסמה קצרה מדי", "hasło jest za krótkie", "a senha é muito curta", "la contraseña es demasiado corta", "Code accès trop court", "パスワードが短すぎます", "la password è troppo corta"},
		{"password is too long", "密码太长", "Passwort ist zu lang", "סיסמה ארוכה מדי", "hasło jest za długie", "A senha é muito longa", "La contraseña es demasiado larga", "Mot de passe trop long", "パスワードが長すぎます", "la password è troppo lunga"},
		{"wrong password", "密码错误", "falsches Passwort", "סיסמה שגויה", "złe hasło", "senha incorreta", "contraseña incorrecta", "Mot de passe incorrect", "パスワードが間違っています", "password errata"},
		{"Please enter Wifi name", "请输入Wifi账号", "Bitte geben Sie den WLAN-Namen ein", "בבקשה הכנס שם WIFI", "Wprowadź nazwę Wi-Fi", "Por favor, insira o nome WiFi", "Por favor ingrese el nombre de wifi", "Saisir nom réseau WIFI (SSID)", "Wi-Fiネットワーク名を入力してください", "Inserisci il nome Wi-Fi"},
		{"Current Door1", "当前门口机1", "Aktuelle TÜR1", "דלת נוכחית 1", "Aktualne Drzwi 1", "Porta atual1", "Puerta actual1", "Platine actuelle 1", "現在の玄関ドアフォン1", "Porta corrente1"},
		{"Door1", "门口机1", "Türstation 1", "דלת  1", "Drzwi 1", "Porta1", "Puerta1", "Platine 1", "玄関ドアフォン1", "Porta 1"},
		{"Current Door2", "当前门口机2", "Aktuelle TÜR2", "דלת נוכחית 2", "Aktualne Drzwi 2", "Porta atual2", "Puerta actual2", "Platine actuelle 2", "現在の玄関ドアフォン2", "Porta corrente2"},
		{"Door2", "门口机2", "Türstation 2", "דלת  2", "Drzwi 2", "Porta2", "Puerta2", "Platine 2", "玄関ドアフォン2", "Porta 2"},
		{"Current Camera1", "当前摄像机1", "Aktuelle Kamera1", "מצלמה נוכחית 1", "Aktualna kamera1", "Câmera atual1", "Cámara actual1", "Caméra actuelle 1", "現在の防犯カメラ1", "Fotocamera corrente1"},
		{"Camera1", "摄像机1", "Kamera 1", "מצלמה 1", "Kamera 1", "Câmera1", "Cámara1", "Caméra 1", "防犯カメラ1", "Fotocamera1"},
		{"Current Camera2", "当前摄像机2", "Aktuelle Kamera2", "מצלמה נוכחית 2", "Aktualna kamera2", "Câmera atual2", "Cámara actual2", "Caméra actuelle 2", "現在の防犯カメラ2", "Fotocamera corrente2"},
		{"Camera2", "摄像机2", "Kamera 2", "מצלמה 2", "Kamera 2", "Câmera2", "Cámara2", "Caméra 2", "防犯カメラ2", "Fotocamera2"},
		{"NO.", "序号", "Nein.", "מספר", "Nie.", "Não.", "No.", "No.", "NO.", "NO."},
		{"Door1 card management", "门1卡片管理", "Kartenverwaltung Tür1", "ניהול כרטיסים דלת 1", "Zarządzanie kartami Drzwi 1", "Gestão de cartões Portão 1", "Puerta 1 Gestión de tarjetas", "Portes 1 Gestion des cartes", "門扉1カード管理", "Gestione della carta Porta1"},
		{"Door2 card management", "门2卡片管理", "Kartenverwaltung Tür2", "ניהול כרטיסים דלת 2", "Zarządzanie kartami Drzwi 2", "Gestão de cartões Portão 2", "Puerta 2 Gestión de tarjetas", "Portes 2 Gestion des cartes", "門扉1カード管理", "Gestione della carta Porta2"},
		{"Door1 fingerprint management", "门1指纹管理", "Tür1 Fingerabdruckverwaltung", "ניהול טביעות אצבעות שער 1", "Drzwi1 zarządzanie odciskami palców", "Porta 1 Gestão de impressões digitais", "Puerta 1 Gestión de huellas dactilares", "Porte 1 Empreinte digitale", "門扉1指紋管理", "Gestione impronta porta1"},
		{"Door2 fingerprint management", "门2指纹管理", "Tür2 Fingerabdruckverwaltung", "ניהול טביעות אצבעות שער 2", "Drzwi2 zarządzanie odciskami palców", "Porta 2 Gestão de impressões digitais", "Puerta 2 Gestión de huellas dactilares", "Porte 2 Empreinte digitale", "門扉1指紋管理", "Gestione impronta porta2"},
		{"Door2 password management", "门1密码管理", "Erfolg hinzufügen", "ניהול סיסמאות דלת 1", "Dodaj sukces", "Adicionar Sucesso", "Se agregó con éxito", "Ajouté avec succès", "追加成功", "Aggiungi successo"},
		{"Door2 password management", "门2密码管理", "Erfolg hinzufügen", "ניהול סיסמאות דלת 2", "Dodaj sukces", "Adicionar Sucesso", "Se agregó con éxito", "Ajouté avec succès", "追加成功", "Aggiungi successo"},
		{"Cards managment", "卡片管理", "Kartenverwaltung", "ניהול כרטיסים", "Zarządzanie kartą", "Gestão de cartões", "Gestión de tarjetas", "Gestion des cartes", "カード管理", "Gestione della carta"},
		{"Fingermark manage", "指纹管理", "Fingerabdruck verwalten", "מנהל סימני אצבע", "Zarządzanie znakiem palca", "Gestão de marcas digitais", "Gestión de huellas dactilares", "Gestion Empreinte digitale", "指紋管理", "Gestisci impronte digitali"},
		{"Password manage", "密码管理", "Kartenverwaltung", "ניהול סיסמאות", "Zarządzanie kartą", "Gestão de cartões", "Gestión de tarjetas", "Gestion des cartes", "カード管理", "Gestione della carta"},
		{"Add Success", "添加成功", "Erfolg hinzufügen", "נוסף בהצלחה", "Dodaj sukces", "Adicionar Sucesso", "Se agregó con éxito", "Ajouté avec succès", "追加成功", "Aggiungi successo"},
		{"Add failed", "添加失败", "Hinzufügen fehlgeschlagen", "הוספה נכשלה", "Dodanie nie powiodło się", "A adição falhou", "Falló la Adición", "L'ajout a échoué", "追加に失敗", "Aggiunta non riuscita"},
		{"Insufficient password digits", "密码位数不够", "Unzureichende Kennwortziffern", "מספר סיסמה לא מספיק", "Niewystarczające cyfry hasła", "Números insuficientes da senha", "Número de contraseña insuficiente", "Nombre insuffisant de mots de passe", "パスワードの桁数が不足しています", "Cifre password insufficienti"},
		{"Card number", "卡号", "Kartennummer", "מספר כרטיס", "Numer karty", "Número do cartão", "Número de tarjeta", "Numéro de carte", "カード番号", "Numero di carta"},
		{"Fingerprint number", "指纹号码", "Fingerabdrucknummer", "מספר טביעת אצבע", "Numer odcisku palca", "Número da impressão digital", "Número de huella dactilar", "Numéro d'empreinte digitale", "指紋番号", "Numero di impronte digitali"},
		{"Password", "密码", "Kartennummer", "מספר סיסמה", "Numer karty", "Número do cartão", "Número de tarjeta", "Numéro de carte", "カード番号", "Numero di carta"},
		{"Please add fingerprint", "请添加指纹", "Bitte Fingerabdruck hinzufügen", "בבקשה תוסיף טביעת אצבע", "Proszę dodać odcisk palca", "Por favor, adicione impressões digitais", "Por favor, agregue una huella dactilar", "Añada Huellas numériques", "指紋を追加してください", "Aggiungi l'impronta digitale"},
		{"Please delete fingerprint", "请删除指纹", "Bitte löschen Sie Fingerabdruck", "אנא מחק טביעת אצבע", "Proszę usunąć odcisk palca", "Por favor, apague a impressão digital", "Por favor, elimine las huellas dactilares", "Veuillez supprimer les empreintes digitales", "指紋を削除してください", "Si prega di eliminare l'impronta digitale"},
		{"Please add a card", "请添加卡片", "Bitte fügen Sie eine Karte hinzu", "בבקשה הוסף כרטיס", "Proszę dodać kartę", "Por favor, adicione um cartão", "Por favor, agregue la tarjeta", "Veuillez ajouter une carte", "カードを追加してください", "Si prega di aggiungere una carta"},
		{"Please delete the card", "请删除卡片", "Bitte löschen Sie die Karte", "בבקשה מחק את הכרטיס", "Proszę usunąć kartę.", "Por favor, apague o cartão", "Por favor, elimine la tarjeta", "Veuillez supprimer la carte", "カードを削除してください", "Si prega di cancellare la carta"},
		{"Please add aPassword", "请添加密码", "Kartennummer", "בבקשה הוסף סיסמה", "Numer karty", "Número do cartão", "Número de tarjeta", "Numéro de carte", "カード番号", "Numero di carta"},
		{"Please delete thePassword", "请删除密码", "Kartennummer", "בבקשה מחק סיסמה", "Numer karty", "Número do cartão", "Número de tarjeta", "Numéro de carte", "カード番号", "Numero di carta"},
		{"Illegal operation", "非法操作", "Illegaler Betrieb", "פעולה לא חוקית", "Nielegalne działanie", "Operação ilegal", "Operaciones ilegales", "Opérations illégales", "不正な操作", "Operazione illegale"},
		{"The number of passwords is full", "密码数量已满", "Die Anzahl der Passwörter ist voll", "מספר הסיסמאות מלא", "Liczba haseł jest pełna", "O número de senhas está cheio", "El número de contraseñas está lleno", "Le nombre de mots de passe est complet", "パスワード数がいっぱいです", "Il numero di password è pieno"},
		{"Existing equipment online", "已有设备在线", "Vorhandene Geräte online", "ציוד קיים באינטרנט", "Istniejące urządzenia online", "Equipamento existente em linha", "Dispositivos existentes en línea", "Équipement déjà en ligne", "既存のデバイスはオンライン", "Dispositivo esistente online"},

		{"Heavy rain", "大雨", "Starker Regen", "גשם כבד", "silny deszcz", "chuva forte", "Fuertes lluvias", "Fortes pluies", "大雨", "pioggia pesante"},
		{"Thunderstorm", "雷暴", "thunderstorm", "רעם", "Burza z piorunami", "Trovoada", "Tormenta eléctrica", "Orage", "らいあらし", "Temporali"},
		{"Sandstorm", "沙尘暴", "Sandsturm", "סערת חול", "burza piaskowa", "tempestade de areia", "Tormenta de arena", "Tempête de sable", "砂嵐", "tempesta di sabbia"},
		{"Light snow", "小雪", "Leichter Schneefall", "שלג קטן", "Mały śnieg", "Neve Minorweather condition", "Nieve ligera", "Petite neige", "小雪", "Neve minore"},
		{"Snow", "雪", "Schnee", "שלג", "śnieg", "neve", "Nieve", "Neige", "雪", "neve"},
		{"freezing fog", "冻雾", "Gefriernebel", "ערפל קפוא", "Mrożona mgła", "Nevoeiro congelado", "Niebla helada", "Brouillard gelé", "とうけつ霧", "Nebbia congelata"},
		{"Rainstorm", "暴雨", "Regenweather condition", "גשם", "Burza", "Tempestade", "Lluvias torrenciales", "Pluie torrentielle", "豪雨", "Pioggia"},
		{"partial showers", "局部阵雨", "Teilduschen", "מקלחות בודדות", "Przelotne opady", "chuveiros isolados", "Chubascos locales", "Averses locales", "局地にわか雨", "docce isolate"},
		{"dust", "浮尘", "Staub", "אבק צף", "Pył", "Poeira flutuante", "Polvo flotante", "La poussière flottante", "ちり", "Polvere galleggiante"},
		{"thunderbolt", "雷电", "Blitz", "רעם", "Piorun", "trovão", "Truenos y relámpagos", "La foudre", "雷", "Tuono"},
		{"Light showers", "小阵雨", "Leichte Schauer", "מקלחת קלה.", "Lekkie opady", "Um chuveiro leve", "Pequeñas lluvias", "Petites averses de pluie", "小雨", "Una doccia leggera"},
		{"Rain", "雨", "Regen", "גשם", "Deszcz", "Chuva", "Lluvia", "La pluie", "雨", "Pioggia"},
		{"sleet", "雨夹雪", "Schneeregen", "סלט", "Deszcz ze śniegiem", "sleet", "Aguanieve", "Grésillement de pluie", "みぞれ", "sleet"},
		{"Dust tornado", "尘卷风", "Staubtornado", "סערת חול", "Tornado", "Tempestade de areia", "Viento de arena y polvo", "Vent de poussière de sable", "砂塵風", "Diavolo della polvere"},
		{"ice grains", "冰粒", "Eiskörner", "חלקיקי קרח", "Grad", "Partículas de gelo", "Granos de hielo", "Grains de glace", "ひょうりゅう", "Particelle di ghiaccio"},
		{"Strong sandstorm", "强沙尘暴", "Starker Sandsturm", "סערת חול חזקה", "Silna burza piaskowa", "Tempestade de areia forte", "Una fuerte tormenta de arena", "Forte tempête de sable", "強い砂嵐", "Forte tempesta di sabbia"},
		{"Blowing sand", "扬沙", "Blassand", "תרים חול", "Jansa", "Levantar areia", "Yangsha", "Yansha", "サンドブラスト", "Yangsha"},
		{"Light to moderate rain", "小到中雨", "Leichter bis mäßiger Regen", "גשם קל עד מוצג", "Umiarkowany deszcz", "Chuva leve a moderada", "Lluvia ligera a moderada", "Pluie faible à modérée", "小雨から中雨まで", "Pioggia leggera a moderata"},
		{"clear", "大部晴朗", "klar", "בעיקר שמש", "Przejrzyście", "Sobretudo ensolarado.", "La mayor parte despejada", "Grand clair", "大部晴晴", "Per lo più soleggiato"},
		{"Sunny", "晴", "Sonnig", "נקה", "Słonecznie", "Limpar", "Soleado", "Ensoleillé", "晴れ", "Soleggiato"},
		{"Fog", "雾", "Nebel", "ערפל", "Mgła", "Nevoeiro", "Brumas", "Brouillard", "霧", "Nebbia"},
		{"Shower", "阵雨", "Dusche", "מקלחת", "Opady", "chuveiro", "Chubascos", "Averses de pluie", "にわか雨", "doccia"},
		{"Heavy showers", "强阵雨", "Starke Schauer", "מקלחות כבדות", "Mocne opady", "Chuveiro forte", "Fuertes lluvias", "Fortes averses", "強いにわか雨", "Pioggia forte"},
		{"Heavy snow", "大雪", "Starker Schneefall", "שלג כבד", "Mocny śnieg", "Major Snow", "Nieve pesada", "Grosse neige", "大雪", "Neve maggiore"},
		{"Torrential rain", "特大暴雨", "Extrem starker Regensturm", "סערת גשם כבדה ביותר", "Niezwykle silna burza deszczowa", "Tempestade extremamente forte", "Fuertes lluvias", "Très fortes pluies", "ゲリラ豪雨", "Pioggia estremamente forte"},
		{"Blizzard", "暴雪", "Schneesturm", "בליזארד", "Burza śnieżna", "Blizzard", "Tormenta de nieve", "Blizzard", "大雪", "La bufera di neve"},
		{"Hail", "冰雹", "Hagel", "קרח", "Grad", "granizo", "Granizo", "Grêle", "雹", "grandine"},
		{"Light to moderate snow", "小到中雪", "Leichter bis mäßiger Schnee", "שלג קל עד ממוצע", "Umiarkowany śnieg", "Neve leve a moderada", "Nieve ligera a moderada", "Neige petite à moyenne", "小雪から中雪まで", "Neve leggera a moderata"},
		{"Partly cloudy", "部分多云", "Teilweise bewölkt", "ענן חלקי", "Częściowe zachmurzenie", "Parcialmente nublado", "Parcialmente nublado", "Partiellement nuageux", "部分的に曇りが多い", "Parzialmente nuvoloso"},
		{"Small snow showers", "小阵雪", "Kleine Schneeschauer", "זחלה", "Słaby śnieg", "flurry", "Pequeñas chubascos de nieve", "Petites averses de neige", "小雪", "flurry"},
		{"medium snow", "中雪", "mittlerer Schnee", "שלג ממוצע", "Średni śnieg", "neve moderada", "Nieve moderada", "Moyenne neige", "普雪", "neve moderata"},
		{"Overcast", "阴", "Bewölkt", "גזע", "Zachmurzenie", "Nublado", "Nublado", "Jours nuageux", "陰天", "Coperto"},
		{"ice needle", "冰针", "Eisnadel", "מחט קרח", "Sople", "Agulha de gelo", "Aguja de hielo", "Aiguilles à glace", "アイスニードル", "Ago di ghiaccio"},
		{"Rainstorm", "大暴雨", "Starker Regenschauer", "סערת גשם כבדה", "Silna burza deszczowa", "Tempestade forte", "Fuertes lluvias", "Fortes pluies", "豪雨", "Pioggia forte"},
		{"Thunderstorms with hail", "雷阵雨伴有冰雹", "Gewitter mit Hagel", "סופת רעמים עם ברד", "Burza z piorunami i gradem", "Chuva de trovão com granizo", "Tormentas eléctricas acompañadas de granizo", "Orage avec grêle", "夕立に雹が伴う", "Pioggia di tuoni accompagnati da grandine"},
		{"Freezing rain", "冻雨", "Eisregenweather condition", "גשם קפוא", "Marznący deszcz", "Chuva gelada", "Lluvia helada", "Pluie verglaçante", "冷たい雨", "Pioggia congelante"},
		{"Snow showers", "阵雪", "Schneeschauer", "מקלחת שלג", "Opady śniegu", "Chuva de neve", "Chubascos de nieve", "Averses de neige", "にわか雪", "Nevicata"},
		{"Light rain", "小雨", "leichter Regen", "גשם קל", "Lekki deszcz", "chuva leve", "Lluvia ligera", "Petite pluie", "小雨", "pioggia leggera"},
		{"haze", "霾", "Dunst", "היי", "Mgła", "Neblina", "Neblina", "La brume", "スモッグ", "Haze"},
		{"Moderate rain", "中雨", "Mäßiger Regen", "גשם מוצג", "Umiarkowane opady", "chuva moderada", "Lluvia moderada", "Pluie modérée", "普雨", "pioggia moderata"},
		{"Partly cloudy", "多云", "Teilweise bewölkt", "ענן", "Częściowe zachmurzenie", "Nublado", "Nublado", "Nuageux", "曇りが多い", "Nuvoloso"},
		{"Thunderstorm", "雷阵雨", "Gewitterschauer", "סופת רעמים", "Burza z piorunami", "Chuva de trovoadas", "Tormenta eléctrica", "Averses de tonnerre", "夕立", "Acquazzone"},
		{"Moderate to heavy rain", "中到大雨", "Mäßiger bis starker Regen", "גשם ממוצע עד כבד", "Przejściowe do mocnych opadów", "Chuva moderada a intensa", "Lluvias moderadas a fuertes", "Pluie modérée à forte", "大雨になる", "Pioggia moderata o pesante"},
		{"Rainstorm", "大到暴雨", "Stark bis Regenschauer", "סערת גשם כבדה", "Silne do deszczowej burzy", "Pesada a tempestade", "Lluvias torrenciales", "Grande à forte pluie", "豪雨になる", "Pioggia pesante"},
		{"Sunny", "晴朗", "Sonnig", "שמש", "Słonecznie", "Ensolarado", "Despejado", "Clair", "からりと開けた", "Soleggiato"},

		{"weather switch", "天气开关", "Wetterschalter", "מתג מזג האוויר", "Pogoda", "Botão Meteo", "Botón Meteo", "Switch météo", "天候スイッチ", "interruttore meteo"},
		{"Weather Schedule", "天气持续时间", "Czas trwania pogody", "משך מזג האוויר", "Pogoda Harmonogram", "Duração climática", "Duración del clima", "Horaire météo ", "天候持続時間", "Durata del tempo"},
		{"pressure", "压力", "pressure", "pressure", "Ciśnienie", "pressure", "pressure", "pressure", "あつりょく", "pressure"},
		{"humidity", "湿度 ", "humidity", "humidity", "Wilgotność", "humidity", "humidity", "humidity", "しつど", "humidity"},
		{"PM10", "PM10 ", "PM10", "PM10", "PM10", "PM10", "PM10", "PM10", "PM10", "PM10"},
		{"PM2.5", "PM2.5 ", "PM2.5", "PM2.5", "PM2.5", "PM2.5", "PM2.5", "PM2.5", "PM2.5", "PM2.5"},
		{"Sun.", "星期日", "So.", "ראשון", "Niedz.", "Domingo", "Dom.", "Dim.", "日曜日", "Domenica"},
		{"Mon.", "星期一", "Mo.", "שני", "Pon.", "Segunda", "Lun.", "Lun. ", "月曜日", "Lunedi"},
		{"Tues.", "星期二", "Di.", "שלישי", "Wt.", "Terça", "Mar.", "Mar.", "火曜日", "Martedì"},
		{"Wed.", "星期三", "Mi.", "רביעי", "Śr.", "Quarta", "Mié", "Mer.", "水曜日", "Mercoledì"},
		{"Thur.", "星期四", "Do.", "חמישי", "Czw.", "Quinta", "Jue.", "Jeu.", "木曜日", "Giovedi"},
		{"Fri.", "星期五", "Fr.", "שישי", "Pt.", "Sexta", "Vie.", "Ven.", "金曜日", "Venerdì"},
		{"Sat.", "星期六", "Sa.", "שבת", "Sob.", "Sábado", "Sáb.", "Sam.", "土曜日", "Sabato"},

		// #ifdef DHCP_IPCAMERA
		{"registered", "已注册", "Registriert", "רשום", "Zarejestrowane", "Registado", "Registrado", "Déjà enregistré", "登録済み", "Registrato"},
		{"Available devices", "可用设备", "Verfügbare Geräte", "מכשירים זמינים", "Dostępne urządzenia", "Dispositivos disponíveis", "Dispositivos disponibles", "Équipement disponible", "利用可能なデバイス", "Dispositivi disponibili"},
		{"Replace device", "替换设备", "Gerät ersetzen", "החלף מכשיר", "Wymień urządzenie", "Substituir o dispositivo", "Reemplazar el equipo", "Équipement de remplacement", "代替デバイス", "Sostituisci dispositivo"},
		{"Confirm cancellation", "确认注销", "Stornierung bestätigen", "אישור ביטול", "Potwierdź anulowanie", "Confirmar o cancelamento", "Confirmación de la cancelación", "Confirmer la déconnexion", "ログアウトの確認", "Conferma cancellazione"},
		{"Link device failed", "链接设备失败", "Verbindungsgerät fehlgeschlagen", "הקישור למכשיר נכשל", "Połączenie urządzenia nie powiodło się", "Ligar o dispositivo falhou", "Falló el dispositivo de enlace", "Le périphérique lié a échoué", "リンクデバイスに失敗しました", "Dispositivo di collegamento non riuscito"},
		{"Registered with the same device", "已注册相同设备", "Registriert mit demselben Gerät", "רישום עם אותו מכשיר", "Zarejestrowane na tym samym urządzeniu", "Registado com o mesmo dispositivo", "Se ha registrado el mismo dispositivo", "Le même appareil est déjà enregistré", "同じデバイスが登録されています", "Registrato con lo stesso dispositivo"},
		{"register", "注册", "registrieren", "הרשם", "rejestruj", "registo", "Registro", "Enregistrement", "登録", "registro"},
		{"cancellation", "注销", "Stornierung", "ביטול", "anulowanie", "Cancelamento", "Cancelación", "Déconnexion", "ログアウト", "cancellazione"},
		{"Please enter IPC username", "请输入IPC用户名", "Bitte geben Sie IPC Benutzernamen ein", "אנא הכנס שם משתמש IPC", "Proszę wprowadzić nazwę użytkownika IPC", "Indique por favor o nome de utilizador do IPC", "Introduzca el nombre de usuario del IPC", "Veuillez entrer un nom d'utilisateur IPC", "IPCユーザー名を入力してください", "Inserisci nome utente IPC"},
		{"Please enter the IPC password", "请输入IPC密码", "Bitte geben Sie das IPC-Passwort ein", "אנא הכנס את הסיסמה IPC", "Proszę wprowadzić hasło IPC", "Indique por favor a senha do IPC", "Introduzca la contraseña IPC", "Veuillez entrer le mot de passe IPC", "IPCパスワードを入力してください", "Inserisci la password IPC"},
		{"Webcam", "网络摄像机", "Webcam", "מצלמה אינטרנטית", "Kamera internetowa", "Webcam", "Cámara de red", "Webcam", "Webカメラ", "Webcam"},
		// #endif

		// #ifdef MACHINE_CHIME
		{"Chime Type", "门钟类型", "Art der Türklingel", "סוג פעמון", "Rodzaj gongu", "Tipo de campainha", "Tipo de campana", "Type d'horloge ", "クロックタイプ", "Tipo di campanello"},
		{"Mechanical Chime", "机械门钟", "mechanische Türklingel", "פעמון מכני", "Mechaniczny", "campainha mecânica", "Campanas mecánicas", "Horloge mécanique ", "機械時計", "Carillon meccanico"},
		{"Electronic Chime", "电子门钟", "elektronische Türklingel", "פעמון אלקטרוני", "Elektroniczny", "campainha eletrónica", "Zumbador electrónico", "Buzzer électronique ", "電子ブザー", "Campanello elettronico"},
		// #endif
		{"load...", "加载中...", "laden...", "טען...", "Ładuj...", "carregar...", "Carga", "La charge ", "ふか", "load..."},
		{"Alarm 1", "报警1", "Alarm 1", "אזעקה 1", "Alarm 1", "Alarme 1", "Alarma 1", "Alarme 1 ", "CAM 1アラーム", "Allarme 1"},
		{"Alarm 2", "报警1", "Alarm 2", "אזעקה 2", "Alarm 2", "Alarme 2", "Alarma 2", "Alarme 2 ", "CAM 1アラーム", "Allarme 2"},
		{"light control", "灯控开关", "Lichtsteuerung", "שליטה על התאורה", "kontrola światła", "controlo da luz", "Control de luces", "Contrôle de la lumière", "ライトコントロール", "controllo della luce"},
		{"System is busy", "系统繁忙", "Hier ist viel arbeit.", "המערכת עמוסה", "System jest zajęty", "O sistema está ocupado", "Sistema ocupado", "Le système est occupé", "システムがビジー", "Systém je zaneprázdněn"},
		{"exit button", "出门按钮", "Knopf raus", "לחצן 'יציאה'", "Przycisk Exit", "Botão de saída", "Botón de salida", "Bouton de sortie", "終了ボタン", "Pulsanti di uscita"},

		{"NULL.", "NULL", "NULL", "NULL", "NULL.", "NULL", "NULL.", "NULL.", "NULL", "NULL"},
		{"NULL.", "NULL", "NULL", "NULL", "NULL.", "NULL", "NULL.", "NULL.", "NULL", "NULL"},
};
#endif
void printf_str(void)
{
	printf("\n\n##################################\n");
	for (int i = 0; i < STR_TOTAL; i++)
	{
		printf("%s\n", text_str(i));
	}
	printf("##################################\n\n\n");
}
#ifdef BCOM_OID_VERSION
void *btn_str(enum btn_string_id str_id)
{
	return (void *)multi_lingual[str_id];
}
#endif

static bool lang_xls_import_success_flag = false; // 语言表格倒入成功标志
char *text_str(enum btn_string_id str_id)
{
	if (lang_xls_import_success_flag)
	{
		return lang_xls_str_get(str_id, user_data_get()->language.index);
	}
	else
	{
#ifdef BCOM_OID_VERSION
		return (char *)multi_lingual[str_id][user_data_get()->language.index];
#else
		return "NULL";
#endif
	}
}

int language_total_get(void)
{
	if (lang_xls_import_success_flag)
	{
		return lang_xls_language_num_get();
	}
	else
	{
		return LANGUAGE_TOTAL;
	}
}

int get_curr_data_week(void)
{
	time_t seconds = time(NULL);
	struct tm tm = {0};
	localtime_r(&seconds, &tm);
	return tm.tm_wday;
}

/* 时间显示格式
长日期
中文	2023年2月24日 星期五
英文	Fri. 24/02/2023

 */

#if 0
char *data_fmt_string_get(struct tm *time)
{
	static char buf[32] = {0};
	memset(buf, 0, sizeof(buf));
	int   year = time->tm_year + 1900;
	int   mmon = time->tm_mon + 1;
	int   mday = time->tm_mday;
	char *week = lan_str_get(time->tm_wday + STR_WEEK_SUN);

	switch (user_data_get()->language.index)
	{
	case ENGLISH:
		sprintf(buf, "%s %02d/%02d/%04d", week, mmon, mday, year);
		break;

	// case LANGUAGE_CHINESE:
	
	default:
		sprintf(buf, "%04d/%02d/%02d %s", year, mday, mmon, week);
		break;
	}

	// LOG_YELLOW("date fmt:%s\n", buf);



	return buf;
}
#endif
#define STANDARD_SIZE1 22 // 23
#define STANDARD_SIZE2 20 // 22

static lv_event_info *lv_os_event_queue_node_new(void)
{
	lv_event_info *node = NULL;
	ak_thread_mutex_lock(&lv_os_event_queue_free_mutex);
	if (queue_empty(&lv_os_event_queue_free) == 0)
	{
		node = (lv_event_info *)queue_delete_next(&lv_os_event_queue_free);
	}
	ak_thread_mutex_unlock(&lv_os_event_queue_free_mutex);
	return node;
}

static void lv_os_event_queue_node_del(lv_event_info *node)
{
	if (node != NULL)
	{
		node->msg.type = OS_EVENT_TYPE_NONE;
		node->msg.arg1 = node->msg.arg2 = 0;
		ak_thread_mutex_lock(&lv_os_event_queue_free_mutex);
		queue_insert((queue_s *)node, &lv_os_event_queue_free);
		ak_thread_mutex_unlock(&lv_os_event_queue_free_mutex);
	}
}

static void lv_os_event_handle(const event_msg *msg)
{
	extern unsigned long long os_get_ms(void);
	// unsigned long long x = os_get_ms();
	// printf("lv_os_event_handle event type:%d  START\n",msg->type);
	switch (msg->type)
	{
	case OS_EVENT_TYPE_RECORD:
		if ((msg->arg1 == 1) && (snap_event_callback != NULL))
		{
			snap_event_callback(msg->arg1, msg->arg2);
		}
		else if ((msg->arg1 == 2) && (record_video_event_callback != NULL))
		{
			record_video_event_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_SD:
		if (sd_event_callback != NULL)
		{
			sd_event_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_DEVICE_REPEAT:
		if (device_id_repeat_callback != NULL)
		{
			device_id_repeat_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_INTERPHONE:
		if (interphone_call_callback != NULL)
		{
			interphone_call_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_RING:
		if (msg->arg1 != 0)
		{
			typedef void (*func_callback)(void);
			func_callback func = (func_callback)msg->arg1;
			func();
		}
		break;

	case OS_EVENT_TYPE_UPGRADE:
		if (upgrade_callback != NULL)
		{
			upgrade_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_OUTDOOR_CALL:
		if (monitor_call_callback != NULL)
		{
			monitor_call_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_MOTION_DETECT:
		if (motion_detect_callback != NULL)
		{
			motion_detect_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_INDOOR_CMD:
		if (indoor_cmd_callback != NULL)
		{
			indoor_cmd_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_TUYA:
		if (tuya_event_callback != NULL)
		{
			tuya_event_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_OUTDOOR_STATUS:
		if (info_status_callback != NULL)
		{
			info_status_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_WEATHER_STATUS:
		if (weather_status_callback != NULL)
		{
			weather_status_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_CHIME:
		if (door_chime_callback != NULL)
		{
			door_chime_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_MECHANICAL_KEY:
		if (mechanical_key_callback != NULL)
		{
			mechanical_key_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_ALARM:
		if (alarm_callback != NULL)
		{
			alarm_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_MAILBOX:
		if (mailbox_status_callback != NULL)
		{
			mailbox_status_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_TCP_NETWORK:
		if (tcp_network_callback != NULL)
		{
			tcp_network_callback(msg->arg1, msg->arg2, msg->arg3);
		}
		break;
	case OS_EVENT_TYPE_MONITOR_BUSY:
		if (device_monitor_busy_callback != NULL)
		{
			device_monitor_busy_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_GATE2_UNLOCK:
		if (device_gate2_unlock_callback != NULL)
		{
			device_gate2_unlock_callback(msg->arg1, msg->arg2);
		}
		break;
	case OS_EVENT_TYPE_ADC_KEY_UNLOCK:
		if (device_adc_key_callback != NULL)
		{
			device_adc_key_callback(msg->arg1, msg->arg2);
		}
		break;
	default:
		printf("unknow event type:%d \n", msg->type);
		return;
	}
	// printf("lv_os_event_handle event type:%d  time-consuming:%llu\n",msg->type,os_get_ms() - x);
}

static void lv_os_event_task(struct _lv_task_t *task_t)
{
	lv_event_info *node = NULL;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	if (queue_empty(&lv_os_event_queue_head) == 0)
	{
		node = (lv_event_info *)queue_delete_next(&lv_os_event_queue_head);
	}
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	if (node != NULL)
	{
		event_msg msg = node->msg;
		lv_os_event_queue_node_del(node);
		lv_os_event_handle(&msg);
	}

	extern bool standby_timer_check();
	standby_timer_check();
}

static void lv_os_event_task_init(void)
{
	ak_thread_mutex_init(&lv_os_event_queue_free_mutex, NULL);
	ak_thread_mutex_init(&lv_os_event_queue_head_mutex, NULL);

	queue_initialize(&lv_os_event_queue_head);
	queue_initialize(&lv_os_event_queue_free);
	for (int i = 0; i < OS_EVENT_NUM_MAX; i++)
	{
		queue_insert((queue_s *)&lv_os_event_msg_buffer[i], &lv_os_event_queue_free);
	}

	lv_os_event_ptask = lv_task_create(lv_os_event_task, 30, LV_TASK_PRIO_HIGHEST, NULL);
	lv_task_ready(lv_os_event_ptask);
}

static void layout_init(void)
{

	// 涂鸦使能
	// set_network_tuya_enable(user_data_get()->tuya.enable_tuya);
	/*
		if(user_data_get()->wifi.wifi_open_flag){
			turn_on_wlan_connect();
		}else{
			turn_off_wlan_connect();
		}
		if(user_data_get()->wifi.wifi_open_flag)
			create_check_wlan_task();
	*/

	void find_link_wifi(void);
	find_link_wifi(); // 搜索wifi

	layout_monitor_init();
	layout_call_init();

	tcp_network_info_init();
}

// #include"tuya_uuid_and_key.h"
#define TUYAFILE_PATH "/mnt/tf/tuya_file"

// bool Tuya_uuid_file_read(void)
// {
// 	if(tuya_key_and_uuid_init())
// 	{
// 		return true;
// 	}

// 	if(tuya_uuid_and_key_xls_register(1))
// }
extern bool is_sdcard_insert(void);
bool tuya_uuid_file_read(void)
{

	printf("%s======================================>>>>>>>>>\n\r", __func__);

	char uuid_buffer[25] = {0};
	char key_buffer[32] = {0};
	// 判断sd是否插入 未插入直接返回
	if (is_sdcard_insert() == false)
	{
		return false;
	}

	if (user_data_get()->tuya_info.tuya_uuid[0] != 0 && user_data_get()->tuya_info.tuya_key[0] != 0)
	{
		printf("tuya id :%s\n", user_data_get()->tuya_info.tuya_uuid);
		printf("tuya key :%s\n", user_data_get()->tuya_info.tuya_key);
		return true;
	}

	if (access(TUYAFILE_PATH, F_OK) == -1) // 文件不存在
	{
		return false;
	}
	FILE *fp = fopen(TUYAFILE_PATH, "rw+"); // 可读可写打开文本文件
	if (fp == NULL)
	{
		return false;
	}

	rewind(fp);																								  // 定位流文件头
	if (fscanf(fp, "%s%s", user_data_get()->tuya_info.tuya_uuid, user_data_get()->tuya_info.tuya_key) == EOF) // 取第一行份涂鸦数据
	{
		printf("tuya_file no data OR read file fail....\n\r");
		return false;
	}

	FILE *ff = fopen("/mnt/tf/tuya_cp_file", "w"); // 写打开临时文件tuya_cp_file
	if (fp == NULL)
	{
		printf("creat tuya_cp_file fail !!!!!\n\r ");
	}

	while (fscanf(fp, "%s%s", uuid_buffer, key_buffer) != EOF) // 继续读涂鸦文件直至文件结束或错误
	{
		fprintf(ff, "%s %s\n", uuid_buffer, key_buffer); // 将读到的数据写进临时文件
	}

	printf("tuya id :%s\n", user_data_get()->tuya_info.tuya_uuid);
	printf("tuya key :%s\n", user_data_get()->tuya_info.tuya_key);

	user_data_save();
	fclose(fp);
	fclose(ff);

	ak_sleep_ms(5);
	remove(TUYAFILE_PATH);						   // 删除原涂鸦文件
	rename("/mnt/tf/tuya_cp_file", TUYAFILE_PATH); // 将临时文件重命名
	return true;
}

// static lv_task_t *back_logo_task_t = NULL;
// static void back_logo_task(lv_task_t *task_t)
// {
//     if (back_logo_task_t != NULL)
//     {
//         lv_task_del(back_logo_task_t);
// 		back_logo_task_t = NULL;
//     }

// 	goto_layout(pLAYOUT(standby));

// }

// void goto_logo_display(void)
// {
// 	static rom_bin_info info = rom_bin_info_get(ROM_RES_BG_LOGO_JPG);
// 	system_bg_loading(&info,false);
// 	back_logo_task_t = lv_task_create(back_logo_task, 3000, LV_TASK_PRIO_HIGH, NULL);
// }

static void feed_watchdog_task(lv_task_t *t)
{
	extern void watch_dog_feed(void);
	watch_dog_feed();
}

void leo_api_init(void)
{
	lv_os_event_task_init();

	extern void media_file_list_init(void);
	media_file_list_init();

	extern void video_decode_init(void);
	video_decode_init();

	extern void video_raw_init(void);
	video_raw_init();

	extern void fb_video_mode_enable(bool);
	fb_video_mode_enable(false);

	extern void video_record_init(void);
	video_record_init();

	extern bool audio_play_init(void);
	audio_play_init();

	extern void video_play_init(void);
	video_play_init();

	extern bool network_init(network_device device);
	network_init(user_data_get()->other.network_device);

	if (lang_xls_init(0) == NULL			   // 初始化失败
		|| lang_xls_language_num_get() == 0	   // 语言数量为0
		|| lang_xls_null_str_num_get() > 0	   // 有效区域含有空单元格
		|| lang_xls_str_num_get() < STR_TOTAL) // 字符串数量与UI所需不匹配
	{

		lang_xls_import_success_flag = false;
		Debug("%d  %d   %d\n", lang_xls_language_num_get(), lang_xls_null_str_num_get(), lang_xls_str_num_get());
	}
	else
	{
		lang_xls_import_success_flag = true;
		if (lang_xls_language_num_get() <= user_data_get()->language.index) // 上次保存的语言索引 大于 载入的语言数量（会导致很多地方为空）
		{
			user_data_get()->language.index = 0;
		}
	}

	tuya_language_total_get(LANGUAGE_TOTAL);
	tuya_set_current_language(user_data_get()->language.index);

	// extern void tuya_language_init(char ***first_row_str);
	extern void tuya_language_init_with_param(char ***first_row_str);
	if (lang_xls_import_success_flag)
	{
		tuya_language_init_with_param(lang_xls_a_row_str_get(STR_TUYA_CURR_DOOR1));
	}
#ifdef BCOM_OID_VERSION
	else
	{
		// Debug("============================%d\n\r",__LINE__,tuya_get_current_language);
		// tuya_language_init((char ***)&multi_lingual[STR_TUYA_CURR_DOOR1]);
		extern const char **local_tuya_str;
		local_tuya_str = multi_lingual[STR_TUYA_CURR_DOOR1];
	}
#endif

	static rom_bin_info info = rom_bin_info_get(ROM_RES_BG_BG3_JPG);
	system_bg_loading(&info, false);
	fb_video_mode_enable(false);
	layout_init();
	speak_enable_set(1);

	if (wifi_usb_module_enable())
	{
		printf("TUYA IPC_APP_PID :%s\n", IPC_APP_PID);
		if (tuya_uuid_etc_exist_check() && tuya_conf_uuid_etc_read(&(user_data_get()->tuya_info)) && wifi_usb_module_enable())
		{
			printf("IPC_APP_PID :%s tuya_uuid :%s           tuya_key:%s  \n", IPC_APP_PID, user_data_get()->tuya_info.tuya_uuid, user_data_get()->tuya_info.tuya_key);
			tuya_init_config_t tuya_cfg;
			memset(&tuya_cfg, 0, sizeof(tuya_cfg));
			strncpy(tuya_cfg.pid, IPC_APP_PID, sizeof(tuya_cfg.pid));
			strncpy(tuya_cfg.uuid, user_data_get()->tuya_info.tuya_uuid, sizeof(tuya_cfg.uuid));
			strncpy(tuya_cfg.key, user_data_get()->tuya_info.tuya_key, sizeof(tuya_cfg.key));
			strncpy(tuya_cfg.ver, IPC_APP_VERSION, sizeof(tuya_cfg.ver));
			strncpy(tuya_cfg.net_dev, user_data_get()->pairing_mode == WIRED_NET ? WIRED_DEV : WLAN_DEV, sizeof(tuya_cfg.net_dev));
			strncpy(tuya_cfg.cache_dir, TUYA_CACHE_PATH, sizeof(tuya_cfg.cache_dir));
			strncpy(tuya_cfg.sd_dir, SD_BASE_PATH2, sizeof(tuya_cfg.sd_dir));
			tuya_sdk_init(&tuya_cfg);
		}
	}

	// printf("%s ==============================>>>%d\n\r",__func__,__LINE__);
	// goto_logo_display();
	// extern void SD_card_space_clear(void);
	// SD_card_space_clear();
	extern void watchdog_open(void);
	watchdog_open();
	lv_task_ready(lv_task_create(feed_watchdog_task, 1000, LV_TASK_PRIO_HIGHEST, NULL));
	monitor_channel_set(MON_CH_NONE);
	goto_layout(pLAYOUT(standby));
}

bool record_jpeg_event_push(char record_mode)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_RECORD;
	node->msg.arg1 = 1;
	node->msg.arg2 = record_mode;

	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool record_video_finnish_event_push(char recode_mode)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_RECORD;
	node->msg.arg1 = 2;
	node->msg.arg2 = recode_mode;

	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

void record_jpeg_event_register(event_pro_callback handle)
{
	snap_event_callback = handle;
}

void record_video_event_register(event_pro_callback handle)
{
	record_video_event_callback = handle;
}

bool sdcard_status_change_push(char arg1, char arg2)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_SD;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;

	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

lv_obj_t *sdcard_insert_msg_box = NULL;
static lv_obj_t *sdcard_format_t = NULL;
static void sdcard_insert_msg_btn_up(lv_obj_t *obj)
{
	char *str = lv_label_get_text((lv_obj_t *)sdcard_insert_msg_box->user_data);
	if (sdcard_insert_msg_box != NULL)
	{
		lv_obj_del(sdcard_insert_msg_box);
		sdcard_insert_msg_box = NULL;
	}
	if (str != NULL && memcmp(str, text_str(STR_PLEASE_FORMAT_SD), strlen(text_str(STR_PLEASE_FORMAT_SD))) == 0)
	{
		start_format_sd_card(3);
	}
}

lv_obj_t *sdcard_insert_msgbox_create(char *str)
{

	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);
	// lv_obj_set_id(window_cont,666);

	lv_obj_t *msgbox_cont = lv_cont_create(window_cont, NULL);

	lv_obj_set_pos(msgbox_cont, 350, 187);
	lv_obj_set_size(msgbox_cont, 324, 226);
	lv_obj_set_style_local_bg_color(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00131D));
	lv_obj_set_style_local_bg_opa(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 18);

	lv_obj_t *img = lv_img_create(msgbox_cont, NULL);
	lv_obj_set_pos(img, 52, 42);
	lv_obj_set_size(img, 220, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE2_PNG);
	lv_img_set_src(img, &info1);

	lv_obj_t *window_head_label = lv_label_create(msgbox_cont, NULL);
	lv_label_set_long_mode(window_head_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(window_head_label, str);
	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_align(window_head_label, msgbox_cont, LV_ALIGN_CENTER, 0, -30);
	window_cont->user_data = window_head_label;
	window_head_label->user_data = window_cont;

	lv_obj_t *window_ok_btn = lv_btn_create(msgbox_cont, NULL);
	lv_obj_set_size(window_ok_btn, 160, 48);
	lv_obj_set_style_local_bg_opa(window_ok_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_SETTING_COMFIRM_PNG);

	static btn_data btn_data1 = {0};
	btn_data1.OPS_UP = sdcard_insert_msg_btn_up;
	lv_obj_set_style_local_pattern_image(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info2);
	// lv_obj_set_style_local_pattern_image(window_ok_btn,LV_OBJ_PART_MAIN,LV_STATE_FOCUSED,&info2);
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(255, 0, 0));
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_obj_align(window_ok_btn, msgbox_cont, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
	window_ok_btn->user_data = &btn_data1;
	btn_touch_event_listen(window_ok_btn);

	return window_cont;
}

void setting_sdcard_callback(unsigned long arg1, unsigned long arg2)
{
	extern bool backlight_status_get(void);
	if (backlight_status_get() == false)
		return;

	switch (arg1)
	{
	case 0:
	case 1:
		if (arg1 != 0 && arg2 == 0)
		{
			if (sdcard_format_t == NULL)
			{
				sdcard_format_t = msg_window_create(text_str(STR_SD_SCAN), true);
			}
			break;
		}
		else if (arg1 != 0)
		{
			if (sdcard_format_t)
			{
				lv_obj_del_reload(&(sdcard_format_t)); /* !!! 一定要删除loading 弹窗的父对象 */
			}
		}

		if (format_sd_card_status())
		{
			return;
		}

		if (current_layout_get() == &layout_file_list)
		{
			goto_layout(pLAYOUT(file_list));
		}

		if (sdcard_insert_msg_box == NULL)
		{
			sdcard_insert_msg_box = sdcard_insert_msgbox_create((bool)arg1 ? text_str(STR_INSET_SD_SUCCEE) : text_str(STR_NO_SD_CARD));
		}
		else
		{
			lv_label_set_text((lv_obj_t *)sdcard_insert_msg_box->user_data, (bool)arg1 ? text_str(STR_INSET_SD_SUCCEE) : text_str(STR_NO_SD_CARD));
			lv_obj_align(sdcard_insert_msg_box->user_data, ((lv_obj_t *)sdcard_insert_msg_box->user_data)->user_data, LV_ALIGN_CENTER, 0, -30);
		}
		break;
	case 2:
		if (arg2 == 1)
		{
			Debug("SDCARD START FORMATING.......\n\n");
			if (sdcard_format_t == NULL)
			{
				sdcard_format_t = msg_window_create(text_str(STR_FORMATING), true);
			}
		}
		else if (arg2 == 2)
		{
			Debug("SDCARD START FINISH!!!!!!\n\n");
			if (sdcard_format_t)
			{
				lv_obj_del_reload(&(sdcard_format_t)); /* !!! 一定要删除loading 弹窗的父对象 */
			}

			if (sdcard_insert_msg_box == NULL)
			{
				sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_FORMAT_SUCCE));
			}
			else
			{
				lv_label_set_text((lv_obj_t *)sdcard_insert_msg_box->user_data, text_str(STR_FORMAT_SUCCE));
				lv_obj_align(sdcard_insert_msg_box->user_data, ((lv_obj_t *)sdcard_insert_msg_box->user_data)->user_data, LV_ALIGN_CENTER, 0, -30);
			}
		}
		else if (arg2 == 3)
		{
			if (sdcard_insert_msg_box == NULL)
			{
				sdcard_insert_msg_box = sdcard_insert_msgbox_create(text_str(STR_PLEASE_FORMAT_SD));
			}
			else
			{
				lv_label_set_text((lv_obj_t *)sdcard_insert_msg_box->user_data, text_str(STR_PLEASE_FORMAT_SD));
				lv_obj_align(sdcard_insert_msg_box->user_data, ((lv_obj_t *)sdcard_insert_msg_box->user_data)->user_data, LV_ALIGN_CENTER, 0, -30);
			}
			// #endif
		}
		break;
	default:
		break;
	}
}

void sdcard_event_register(event_pro_callback handle)
{
	sd_event_callback = handle;
}

bool device_id_repeat_push(network_device device)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_DEVICE_REPEAT;
	node->msg.arg1 = device;
	node->msg.arg2 = 0;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

static lv_obj_t *device_id_repeat_msg_obj = NULL;
static void device_id_repeat_msg_btn_up(lv_obj_t *obj)
{
	lv_obj_del(obj);
	device_id_repeat_msg_obj = NULL;
}

static lv_obj_t *device_id_repeat_msg_create(char arg1)
{
	lv_obj_t *msg_box = lv_msgbox_create(lv_scr_act(), NULL);
	lv_obj_set_style_local_bg_color(msg_box, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(57, 57, 57));
	lv_obj_set_style_local_bg_opa(msg_box, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	static const char *btns[] = {"Confirm", ""};
	char buffer[64] = {"0"};
	sprintf(buffer, "\n\n Device ID%d repeat!\n\n", arg1);
	lv_msgbox_set_text(msg_box, buffer);
	lv_msgbox_add_btns(msg_box, btns);
	lv_obj_set_size(msg_box, 400, 300);

	static btn_data btn_data = btn_data_up_create(device_id_repeat_msg_btn_up);
	msg_box->user_data = &btn_data;
	btn_touch_event_listen(msg_box);
	lv_obj_align(msg_box, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_t *btnmatri_btn = lv_msgbox_get_btnmatrix(msg_box);
	lv_obj_set_style_local_bg_color(btnmatri_btn, LV_BTNMATRIX_PART_BTN, LV_STATE_PRESSED, LV_COLOR_MAKE(0xFF, 0, 0));
	lv_obj_set_style_local_radius(btnmatri_btn, LV_BTNMATRIX_PART_BTN, LV_STATE_DEFAULT, 45);
	return msg_box;
}

static void device_id_repeat_callback_default(unsigned long arg1, unsigned long arg2)
{
	if (device_id_repeat_msg_obj == NULL)
	{
		device_id_repeat_msg_obj = device_id_repeat_msg_create((char)arg1);
	}
}

event_pro_callback device_id_repeat_register(event_pro_callback handle)
{
	event_pro_callback old = device_id_repeat_callback;
	device_id_repeat_callback = handle;
	return old;
}

void interphone_call_event_register(event_pro_callback handle)
{
	interphone_call_callback = handle;
}

bool interphone_call_event_push(char arg1, char arg2)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_INTERPHONE;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool ring_play_event_push(unsigned long arg1, unsigned long arg2)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_RING;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

void tuya_event_register(event_pro_callback handle)
{

	tuya_event_callback = handle;
}

void network_event_register(event_pro_callback handle)
{
	network_event_callback = handle;
}

void upgrade_event_register(event_pro_callback handle)
{
	upgrade_callback = handle;
}

bool upgrade_event_push(char arg1, char arg2)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_UPGRADE;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

void outdoor_call_event_register(event_pro_callback handle)
{
	monitor_call_callback = handle;
}

bool outdoor_call_event_push(char arg1, char arg2)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		printf("lv_os_event_queue_node_new fail ...\n\r");
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_OUTDOOR_CALL;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

void motion_detect_event_register(event_pro_callback handle)
{
	motion_detect_callback = handle;
}

bool motion_detect_event_push(char arg1, char arg2)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_MOTION_DETECT;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

void indoor_cmd_event_register(event_pro_callback handle)
{
	indoor_cmd_callback = handle;
}

bool indoor_cmd_event_push(unsigned long arg1, unsigned long arg2)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_INDOOR_CMD;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

void dev_info_status_event_register(event_pro_callback handle)
{
	info_status_callback = handle;
}

void weather_status_event_register(event_pro_callback handle)
{
	weather_status_callback = handle;
}

void door_chime_event_register(event_pro_callback handle)
{
	door_chime_callback = handle;
}

void mechanical_key_event_register(event_pro_callback handle)
{
	mechanical_key_callback = handle;
}

void alarm_event_register(event_pro_callback handle)
{
	alarm_callback = handle;
}

void mailbox_status_event_register(event_pro_callback handle)
{
	mailbox_status_callback = handle;
}

bool dev_info_status_event_push(unsigned long arg1, unsigned long arg2)
{
	Debug("====dev_info_status_event_push=======>>>>arg1:%ld	arg2:%ld\n\n", arg1, arg2);
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_OUTDOOR_STATUS;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool weather_status_event_push(unsigned long arg1, unsigned long arg2)
{
	Debug("====weather_status_event_push=======>>>>arg1:%ld	arg2:%ld\n\n", arg1, arg2);
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_WEATHER_STATUS;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

void tcp_network_event_register(tcp_event_pro_callback handle)
{
	tcp_network_callback = handle;
}
bool tcp_network_event_push(unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
	Debug("====tcp_network_event_push=======>>>>\n\n");
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TCP_NETWORK;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	node->msg.arg3 = arg3;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

void device_monitor_busy_register(event_pro_callback handle)
{
	device_monitor_busy_callback = handle;
}
void device_monitor_busy_push(unsigned long arg1, unsigned long arg2)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return;
	}
	node->msg.type = OS_EVENT_TYPE_MONITOR_BUSY;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return;
}

void default_gate2_unlock_callback(unsigned long arg1, unsigned long arg2)
{
	tuya_ungate2_start();
}

void device_gate2_unlock_register(event_pro_callback handle)
{
	device_gate2_unlock_callback = handle;
}
void device_gate2_unlock_push(unsigned long arg1, unsigned long arg2)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return;
	}
	node->msg.type = OS_EVENT_TYPE_GATE2_UNLOCK;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return;
}

void device_adc_key_register(event_pro_callback handle)
{
	device_adc_key_callback = handle;
}

void adc_key_event_push(unsigned long arg1, unsigned long arg2)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return;
	}
	node->msg.type = OS_EVENT_TYPE_ADC_KEY_UNLOCK;
	node->msg.arg1 = arg1;
	node->msg.arg2 = arg2;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return;
}

/*
 *	涂鸦相关的函数处理
 */

bool tuya_monitor_swap_event(int ch)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_MONITOR_SWAP;
	node->msg.arg2 = ch;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool tuya_enter_monitor_push(void)
{
	Debug("====tuya_enter_monitor_push=======>>>>tuya event");
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_MONITOR_ING;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool tuya_monitor_talk_event(bool state)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_TALK;
	node->msg.arg2 = state;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool tuya_monitor_light_event(bool state)
{
	printf("tuya_monitor_light_event ==========>>>\n\r");
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	printf("TUYA_MONITOR_LIGHT_EVENT ==========>>>%d\n\r", state);
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_LIGHT_ON;
	node->msg.arg2 = state;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool tuya_work_mode_switch_event(int mode)
{
	printf("tuya_work_mode_switch_event ==========>>>\n\r");
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	printf("TUYA_WORK_MODE_SWITCH_EVENT ==========>>>%d\n\r", mode);
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_WORK_MODE;
	node->msg.arg2 = mode;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool tuya_unlock_event(bool state, tuya_event event)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = event;
	node->msg.arg2 = state;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool tuya_monitor_gate2_event(bool state)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_OPEN_GATE2;
	node->msg.arg2 = state;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool tuya_monitor_absent_mode_event(bool state)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_ABSENT_MODE;
	node->msg.arg2 = state;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}
bool tuya_monitor_enter_event(void)
{
	Debug("====tuya_monitor_enter_event=======>>>>tuya event");
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_MONITOR_ENTER;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}
bool tuya_monitor_quit_event(void)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_MONITOR_QUIT;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool tuya_screenshot_event(void)
{
	Debug("====tuya_screenshot_event=======>>>>tuya event");
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_SCREENSHOT;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool tuya_mqtt_offline_event(void)
{
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_TUYA;
	node->msg.arg1 = TUYA_EVENT_MQTT_OFFLINE;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool door_chime_detect_push(void)
{
	Debug("====door_chime_detect=======>>>>\n");
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_CHIME;
	node->msg.arg1 = TUYA_EVENT_DOOR_CHRIME;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool mechanical_key_detect_push(void)
{
	Debug("====mechanical_key_detect=======>>>>t\n");
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_MECHANICAL_KEY;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool alarm_detect_push(void)
{
	Debug("====alarm_detect_push=======>>>>t\n");
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_ALARM;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

bool mailbox_status_detect_push(void)
{
	Debug("====mailbox_status_detect======>>>>");
	lv_event_info *node = lv_os_event_queue_node_new();
	if (node == NULL)
	{
		return false;
	}
	node->msg.type = OS_EVENT_TYPE_MAILBOX;
	node->msg.arg1 = TUYA_EVENT_MAILBOX_STATUS;
	ak_thread_mutex_lock(&lv_os_event_queue_head_mutex);
	queue_insert((queue_s *)node, &lv_os_event_queue_head);
	ak_thread_mutex_unlock(&lv_os_event_queue_head_mutex);
	return true;
}

#ifdef INTERCOM_VERSION
#define ICON_OFFSET (50 + *icon_offset)
#else
#define ICON_OFFSET (424 - *icon_offset)
#endif
void wifi_icon_display(lv_obj_t *parent, int *icon_offset, unsigned long arg1, unsigned long arg2)
{
	lv_obj_t *wifi_icon = NULL;
	if (wifi_usb_module_enable())
	{
		if ((wifi_icon = lv_obj_get_child_form_id(parent, 99)) == NULL)
		{

			if (user_data_get()->wifi.wifi_connect_flag)
			{
				wifi_icon = lv_img_create(parent, NULL);
				lv_obj_set_id(wifi_icon, 99);
				lv_obj_set_pos(wifi_icon, ICON_OFFSET, 34);
				lv_obj_set_size(wifi_icon, 50, 50);
				static rom_bin_info wifi_info = rom_bin_info_get(ROM_RES_SETTING_WIFI_3_PNG);
				lv_img_set_src(wifi_icon, &wifi_info);
				Debug("icon_offset:%d\n\n\n", *icon_offset);
				*icon_offset += 46;
			}
		}
		else
		{

			if (user_data_get()->wifi.wifi_connect_flag && arg1)
			{
				lv_obj_set_pos(wifi_icon, ICON_OFFSET, 34);
				lv_obj_set_hidden(wifi_icon, false);
				*icon_offset += 46;
			}
			else
			{
				lv_obj_set_hidden(wifi_icon, true);
			}
		}
	}
}

void tuya_icon_display(lv_obj_t *parent, int *icon_offset, unsigned long arg1, unsigned long arg2)
{
	lv_obj_t *tuya_icon = NULL;
	if ((tuya_icon = lv_obj_get_child_form_id(parent, 98)) == NULL)
	{
		// Debug("tuya_ipc_register_status_get():%d\n", tuya_ipc_register_status_get());
		if (tuya_online_status_get() == true && (user_data_get()->wifi.wifi_connect_flag || user_data_get()->pairing_mode == WIRED_NET))
		{
			tuya_icon = lv_img_create(parent, NULL);
			lv_obj_set_id(tuya_icon, 98);
			lv_obj_set_pos(tuya_icon, ICON_OFFSET, 32);
			lv_obj_set_size(tuya_icon, 50, 50);
			static rom_bin_info tuya_info = rom_bin_info_get(ROM_RES_TUYA_REGISTER_PNG);
			lv_img_set_src(tuya_icon, &tuya_info);
			*icon_offset += 46;
		}
	}
	else
	{
		//Debug("tuya_ipc_register_status_get():%d,%ld,%d\n", tuya_ipc_register_status_get(), arg1, user_data_get()->wifi.wifi_connect_flag);
		if (tuya_online_status_get() == true && user_data_get()->wifi.wifi_connect_flag && arg1)
		{
			lv_obj_set_pos(tuya_icon, ICON_OFFSET, 32);
			lv_obj_set_hidden(tuya_icon, false);
			*icon_offset += 46;
		}
		else
		{
			lv_obj_set_hidden(tuya_icon, true);
		}
	}
}

void door_icon_display(int door_id, lv_obj_t *parent, int *icon_offset, unsigned long arg1, unsigned long arg2)
{
	Debug("icon_offset:%d\n\n\n", *icon_offset);
	lv_obj_t *door_icon = NULL;

	if ((door_icon = lv_obj_get_child_form_id(parent, door_id ? 96 : 97)) == NULL)
	{
		if (device_online_state_get(door_id ? DEVICE_OUTDOOR_2 : DEVICE_OUTDOOR_1) == true)
		{
			door_icon = lv_img_create(parent, NULL);
			lv_obj_set_id(door_icon, door_id ? 96 : 97);
			lv_obj_set_pos(door_icon, ICON_OFFSET, 30);
			lv_obj_set_size(door_icon, 50, 50);
			static rom_bin_info door1_info = rom_bin_info_get(ROM_RES_NO_DOOR1_PNG);
			static rom_bin_info door2_info = rom_bin_info_get(ROM_RES_NO_DOOR2_PNG);
			lv_img_set_src(door_icon, door_id ? &door2_info : &door1_info);
			*icon_offset += 46;
		}
	}
	else
	{
		if (device_online_state_get(door_id ? DEVICE_OUTDOOR_2 : DEVICE_OUTDOOR_1) == true && arg1)
		{
			lv_obj_set_pos(door_icon, ICON_OFFSET, 30);
			lv_obj_set_hidden(door_icon, false);
			*icon_offset += 46;
		}
		else
		{
			lv_obj_set_hidden(door_icon, true);
		}
	}
}

void dev_info_status_callback(unsigned long arg1, unsigned long arg2)
{
	extern bool backlight_status_get(void);
	if (backlight_status_get() == false)
		return;

	// Debug("====dev_info_status_callback=======>>>>arg1:%lu	arg2:%lu\n\n",arg1,arg2);
	lv_obj_t *info_bar = NULL;
	if ((info_bar = lv_obj_get_child_form_id(lv_scr_act(), 100)) == NULL)
	{
		info_bar = lv_cont_create(lv_scr_act(), NULL);
#ifdef INTERCOM_VERSION
		set_location(info_bar, 0, 0, 512, 100);
#else
		set_location(info_bar, 512, 0, 512, 100);
#endif
		lv_obj_set_id(info_bar, 100);

		if (!arg1)
		{
			lv_obj_set_hidden(info_bar, true);
			return;
		}
	}
	else
	{
		if (!arg1)
		{
			lv_obj_set_hidden(info_bar, true);
			return;
		}
		else
		{
			lv_obj_set_hidden(info_bar, false);
		}
	}

	int icon_offset = 0;
#ifdef INTERCOM_VERSION
	door_icon_display(0, info_bar, &icon_offset, arg1, arg2);
	door_icon_display(1, info_bar, &icon_offset, arg1, arg2);
	tuya_icon_display(info_bar, &icon_offset, arg1, arg2);
	wifi_icon_display(info_bar, &icon_offset, arg1, arg2);
#else
	wifi_icon_display(info_bar, &icon_offset, arg1, arg2);
	tuya_icon_display(info_bar, &icon_offset, arg1, arg2);
	door_icon_display(1, info_bar, &icon_offset, arg1, arg2);
	door_icon_display(0, info_bar, &icon_offset, arg1, arg2);
#endif
}

bool goto_layout(const layout *layout)
{
	// extern unsigned long long os_get_ms(void);
	// unsigned long long x = os_get_ms();

	if ((layout == NULL) || (layout->enter == NULL))
	{
		return false;
	}

	if (sdcard_insert_msg_box != NULL)
	{
		lv_obj_del(sdcard_insert_msg_box);
		sdcard_insert_msg_box = NULL;
	}

	extern void gui_raw_clear(void);
	gui_raw_clear();

	if ((cur_layout != NULL) && (cur_layout->quit != NULL))
	{
		// printf("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
		// x = os_get_ms();
		cur_layout->quit((void *)layout);
		// printf("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
		// x = os_get_ms();
	}

	lv_obj_clean(lv_scr_act());
	device_id_repeat_msg_obj = NULL;

	lv_anim_del_all();

	// lv_img_cache_invalidate_src_all();

	lv_area_t area = {0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX};
	gui_draw_area_set(&area, 1);

	if (cur_layout != layout)
		prev_layout = cur_layout;

	cur_layout = layout;
	// system("echo 3 > /proc/sys/vm/drop_caches");
	// system("sync");
	// printf("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
	// x = os_get_ms();
	layout->enter();
	// printf("%s ==============================>>>%d   %lld\n\r",__func__,__LINE__,os_get_ms() - x);
	// x = os_get_ms();

	return true;
}

bool layout_clear(void)
{
	lv_obj_clean(lv_scr_act());
	device_id_repeat_msg_obj = NULL;
	lv_anim_del_all();
	lv_area_t area = {0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX};
	gui_draw_area_set(&area, 1);

	return true;
}

const layout *current_layout_get(void)
{
	// layout * layout = cur_layout;
	return cur_layout;
}

const layout *prev_layout_get(void)
{
	// layout * layout = cur_layout;
	return prev_layout;
}

static void btn_event_handler(lv_obj_t *obj, lv_event_t event)
{
	btn_data *btn_ev = (btn_data *)obj->user_data;

	if (btn_ev == NULL)
	{
		return;
	}
	// Debug("obj:%p,event:%d\n", obj, event);
	if (event == LV_EVENT_PRESSED)
	{
		extern bool is_monitor_talked(void);
		extern bool get_outdoor_talk_state(MONITOR_CH ch);
		if ((get_outdoor_talk_state(MON_CH_DOOR_1) || get_outdoor_talk_state(MON_CH_DOOR_2)) && current_layout_get() == &layout_standby) // 正在视频对讲其他机子无法操作
		{
		}
		else if ((btn_ev->obj_tone) && (obj != lv_scr_act() || (current_layout_get() == &layout_standby)))
		{
			extern void touch_sound_play(int volume);

			if (!(user_data_get()->audio.key_sound && user_data_get()->other.model != MUTE_PATTERN))
			{
			}

			else
			{
				touch_sound_play(current_layout_get() == &layout_monitor ? (user_data_get()->audio.door_ring_val) : 7);
			}
		}

		if (btn_ev->OPS_DOWN != NULL)
		{
			btn_ev->OPS_DOWN(obj);
		}

		extern bool standby_timer_reset(void);
		standby_timer_reset();
	}
	else if ((event == LV_EVENT_RELEASED) && (btn_ev->OPS_UP != NULL))
	{
		btn_ev->OPS_UP(obj);
	}
	else if (btn_ev->OPS_ANYTHING != NULL)
	{
		btn_ev->OPS_ANYTHING(obj, event);
	}
}

void btn_touch_event_listen(lv_obj_t *obj)
{
	lv_obj_set_event_cb(obj, btn_event_handler);
}

lv_obj_t *prompt_window = NULL;
static void prompt_window_btn_up(lv_obj_t *obj)
{
	if (prompt_window != NULL)
	{
		lv_obj_del(prompt_window);
		prompt_window = NULL;
	}
}

lv_obj_t *prompt_window_create(char *str, void (*up)(lv_obj_t *obj))
{

	lv_obj_t *window_cont = lv_cont_create(lv_scr_act(), NULL);

	lv_obj_set_style_local_bg_opa(window_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
	lv_obj_set_pos(window_cont, 0, 0);
	lv_obj_set_size(window_cont, 1024, 600);
	// lv_obj_set_id(window_cont,666);

	lv_obj_t *msgbox_cont = lv_cont_create(window_cont, NULL);

	lv_obj_set_pos(msgbox_cont, 350, 187);
	lv_obj_set_size(msgbox_cont, 350, 230);
	lv_obj_set_style_local_bg_color(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00131D));
	lv_obj_set_style_local_bg_opa(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_obj_set_style_local_radius(msgbox_cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 18);

	lv_obj_t *img = lv_img_create(msgbox_cont, NULL);
	lv_obj_set_pos(img, 65, 42);
	lv_obj_set_size(img, 220, 2);
	static rom_bin_info info1 = rom_bin_info_get(ROM_RES_SETTING_LINE2_PNG);
	lv_img_set_src(img, &info1);

	lv_obj_t *window_head_label = lv_label_create(msgbox_cont, NULL);
	lv_label_set_long_mode(window_head_label, LV_LABEL_LONG_EXPAND);
	lv_label_set_text(window_head_label, str);

	lv_obj_set_style_local_text_font(window_head_label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(24));
	lv_obj_align(window_head_label, msgbox_cont, LV_ALIGN_CENTER, 0, -30);

	lv_obj_t *window_ok_btn = lv_btn_create(msgbox_cont, NULL);
	lv_obj_set_size(window_ok_btn, 160, 48);
	lv_obj_set_style_local_bg_opa(window_ok_btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

	static rom_bin_info info2 = rom_bin_info_get(ROM_RES_SETTING_COMFIRM_PNG);

	static btn_data btn_data1 = {0};
	btn_data1.obj_tone = true;
	btn_data1.OPS_UP = up ? up : prompt_window_btn_up;
	lv_obj_set_style_local_pattern_image(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, &info2);
	// lv_obj_set_style_local_pattern_image(window_ok_btn,LV_OBJ_PART_MAIN,LV_STATE_FOCUSED,&info2);
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_str(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, text_str(STR_CONFIRM));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 255, 255));
	lv_obj_set_style_local_value_color(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_PRESSED, lv_color_make(255, 0, 0));
	lv_obj_set_style_local_value_align(window_ok_btn, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_value_font(window_ok_btn, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, FONT_SIZE(20));
	lv_obj_align(window_ok_btn, msgbox_cont, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
	window_ok_btn->user_data = &btn_data1;
	btn_data1.user_data = window_cont;
	btn_touch_event_listen(window_ok_btn);

	return window_cont;
}

#include <sys/socket.h>
#include <sys/ioctl.h>

int getmac(char *mac, char *device)
{

	// char *device="eth0"; //eth0是网卡设备名
	unsigned char macaddr[ETH_ALEN]; // ETH_ALEN（6）是MAC地址长度
	struct ifreq req;
	int err, i;
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);	 // internet协议族的数据报类型套接口
	strcpy(req.ifr_name, device);		 // 将设备名作为输入参数传入
	err = ioctl(s, SIOCGIFHWADDR, &req); // 执行取MAC地址操作
	close(s);
	if (err != -1)
	{
		memcpy(macaddr, req.ifr_hwaddr.sa_data, ETH_ALEN); // 取输出的MAC地址
		for (i = 0; i < ETH_ALEN; i++)
		{
			sprintf(mac, "%s%02x", mac, macaddr[i] & 0xff);
			if (i != ETH_ALEN - 1)
			{
				sprintf(mac, "%s:", mac);
			}
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

int net_util_get_ipaddr(char *dev, char *ipaddr)
{
	struct ifreq ifr;
	int fd = 0;
	int ret = -1;
	struct sockaddr_in *pAddr;
	char net_dev[6] = {0};
	strcpy(net_dev, dev);

	if (NULL == ipaddr)
	{
		printf("illegal call function SetGeneralIP!\n");
		return -1;
	}

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("open socket failed\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, net_dev);

	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
	{
		// printf("SIOCGIFADDR socket failed %s ", strerror( errno ));
		close(fd);
		return -1;
	}

	pAddr = (struct sockaddr_in *)&(ifr.ifr_addr);

	strcpy(ipaddr, inet_ntoa(pAddr->sin_addr));

	if (0 == strlen(ipaddr))
	{
		printf("ipaddr = [%s] len = 0\n", ipaddr);
		ret = -1;
	}
	else
	{
		ret = 0;
		// printf( "ipaddr = [%s]\n", ipaddr);
	}
	close(fd);

	return ret;
}

void get_network_time(void)
{
	struct tm local_time;
	bool ret = tuya_net_time_sync(&local_time);
	if (ret == true)
	{
		char date_param[32] = {0};
		sprintf(date_param, "date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",
				local_time.tm_year,
				local_time.tm_mon,
				local_time.tm_mday,
				local_time.tm_hour,
				local_time.tm_min,
				local_time.tm_sec);
		system(date_param);
		system("hwclock -w");

		extern bool standby_timer_reset(void);
		standby_timer_reset();
	}
}

static lv_task_t *tuya_ungate2_task_t = NULL;
static void tuya_ungate2_task(lv_task_t *task_t)
{
	unlock_gpio_set(0);
	tuya_dp_244_response_indoor_gate2(false);
	if (tuya_ungate2_task_t)
	{
		lv_task_del(tuya_ungate2_task_t);
		tuya_ungate2_task_t = NULL;
	}
}
void tuya_ungate2_start(void)
{
	Debug("\n\n");
	if (tuya_ungate2_task_t != NULL)
	{
		return;
	}
	unlock_gpio_set(1);
	tuya_dp_244_response_indoor_gate2(true);
	tuya_ungate2_task_t = lv_task_create(tuya_ungate2_task, user_data_get()->other.unlock_time * 1000, LV_TASK_PRIO_HIGH, NULL);
}

static lv_task_t *tuya_ungate1_task_t = NULL;
static void tuya_ungate1_task(lv_task_t *task_t)
{
	tuya_dp_232_response_outdoor_gate1(false);
	if (tuya_ungate1_task_t)
	{
		lv_task_del(tuya_ungate1_task_t);
		tuya_ungate1_task_t = NULL;
	}
}
void tuya_ungate1_start(void)
{
	Debug("\n\n");
	if (tuya_ungate1_task_t != NULL)
	{
		return;
	}
	tuya_dp_232_response_outdoor_gate1(true);
	network_cmd_data data;
	data.device = monitor_channel_get() == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : monitor_channel_get() == MON_CH_DOOR_2 ? DEVICE_OUTDOOR_2
																													 : DEVICE_UNKONW;
	data.cmd = NET_COMMON_CMD_UNLOCK;
	data.arg1 = monitor_channel_get() == MON_CH_DOOR_2 ? user_data_get()->door2.ungate1_delay : user_data_get()->door1.ungate1_delay;
	data.arg2 = 2 | user_data_get()->language.index << 2 | user_data_get()->other.unlock_hint << 7;
	network_send_cmd_data(&data);
	tuya_ungate1_task_t = lv_task_create(tuya_ungate1_task, (monitor_channel_get() == MON_CH_DOOR_2 ? user_data_get()->door2.ungate1_delay : user_data_get()->door1.ungate1_delay) * 1000, LV_TASK_PRIO_HIGH, NULL);
}

static lv_task_t *tuya_unlock_task_t = NULL;
static void tuya_unlock_task(lv_task_t *task_t)
{
	tuya_dp_234_response_outdoor_lock(false);
	if (tuya_unlock_task_t)
	{
		lv_task_del(tuya_unlock_task_t);
		tuya_unlock_task_t = NULL;
	}
}
void tuya_unlock_start(void)
{
	Debug("\n\n");
	if (tuya_unlock_task_t != NULL)
	{
		return;
	}
	tuya_dp_234_response_outdoor_lock(true);
	network_cmd_data data;
	data.device = monitor_channel_get() == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : monitor_channel_get() == MON_CH_DOOR_2 ? DEVICE_OUTDOOR_2
																													 : DEVICE_UNKONW;
	data.cmd = NET_COMMON_CMD_UNLOCK;
	data.arg1 = monitor_channel_get() == MON_CH_DOOR_2 ? user_data_get()->door2.unlock_delay : user_data_get()->door1.unlock_delay;
	data.arg2 = 1 | user_data_get()->language.index << 2 | user_data_get()->other.unlock_hint << 7;
	network_send_cmd_data(&data);
	tuya_unlock_task_t = lv_task_create(tuya_unlock_task, (monitor_channel_get() == MON_CH_DOOR_2 ? user_data_get()->door2.unlock_delay : user_data_get()->door1.unlock_delay) * 1000, LV_TASK_PRIO_HIGH, NULL);
}

void send_monitor_talk_cmd(bool talk_en)
{
	static MONITOR_CH ch = MON_CH_NONE;
	ch = monitor_channel_get();
	network_cmd_data data;
	data.cmd = NET_COMMON_CMD_OUTDOOR_TALK;
	data.arg1 = talk_en ? ch : 0;				  //
	data.arg2 = user_data_get()->other.family_id; //
	data.device = DEVICE_ALL;
	network_send_cmd_data(&data);
}
void send_monitor_hang_cmd(void)
{
	static MONITOR_CH ch = MON_CH_NONE;
	ch = monitor_channel_get();
	network_cmd_data data;
	data.cmd = NET_COMMON_CMD_OUTDOOR_HANG;
	data.arg1 = ch;								  //
	data.arg2 = user_data_get()->other.family_id; //
	data.device = DEVICE_ALL /* ch == MON_CH_DOOR_1 ? DEVICE_OUTDOOR_1 : DEVICE_OUTDOOR_2 */;
	// Debug("===========================>>data.device:%d\n", data.device);
	network_send_cmd_data(&data);
}

void lv_obj_del_reload(lv_obj_t **obj)
{
	if (*obj)
	{
		lv_obj_del(*obj);
		*obj = NULL;
	}
}

static void Electronic_Chime_Enable(void)
{
	door_chime_event_register(NULL);
	chime_gpio_enable();
}
static void Electronic_Chime_Disable(void)
{
	door_chime_event_register(door_chime_func);
	chime_gpio_disable();
}
void door_chime_func(unsigned long arg1, unsigned long arg2)
{
	Debug("\n\n\n");
	if (user_data_get()->other.model != MUTE_PATTERN)
	{
#ifdef MACHINE_CHIME
		if (user_data_get()->other.chime_type == false)
		{
			Debug("\n\n\n");
			Mechanical_Chime_Enable();
			return;
		}
#endif

		Debug("\n\n\n");
		chime_sound_play(ring_attr.door1.ring, ring_attr.door1.ring_val, Electronic_Chime_Enable, Electronic_Chime_Disable);
	}
}
#ifdef MACHINE_CHIME
static lv_task_t *machine_chime_task_t = NULL;
static bool Mechanical_Chime_run = false;
void machine_chime_task(lv_task_t *task_t)
{

	static bool en = false;
	static int count = 0;

	if ((en = !en))
	{
		chime_gpio_enable();
	}
	else
	{
		chime_gpio_disable();
	}

	if (++count >= 10 || Mechanical_Chime_run == false)
	{
		count = 0;
		if (machine_chime_task_t)
		{
			lv_task_del(machine_chime_task_t);
			machine_chime_task_t = NULL;
			door_chime_event_register(door_chime_func);
			Mechanical_Chime_run = false;
		}
	}
}
void Mechanical_Chime_Enable(void)
{
	if (machine_chime_task_t == NULL)
	{
		Mechanical_Chime_run = true;
		door_chime_event_register(NULL);
		machine_chime_task_t = lv_task_create(machine_chime_task, 1000, LV_TASK_PRIO_HIGH, NULL);
	}
}
void Mechanical_Chime_Disable(void)
{
	if (machine_chime_task_t)
	{
		Mechanical_Chime_run = false;
	}
}

#endif

/*************************************************************************
 * @brief  创建正在加载的弹窗
 * @date   2022-11-09 09:45
 * @author xiaoele
 * @param  str  显示的文字
 * @param  is_loading 加载的动画
 **************************************************************************/
static void msg_window_event_cb(lv_obj_t *obj, lv_event_t event)
{
	// Debug("event:%d=========================>>>>%p %p\n\n",event,obj,sdcard_format_t);
	if (event == LV_EVENT_DELETE)
	{
		// 启用按键操作

		// Debug("=========================>>>>%p %p\n\n",obj,obj->user_data);
		sdcard_format_t = NULL;
	}
}

lv_obj_t *msg_window_create(char *str, bool is_loading)
{
	lv_obj_t *cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(cont, LV_HOR_RES_MAX, LV_VER_RES_MAX); // 一整个屏幕大小
	lv_obj_align(cont, NULL, LV_ALIGN_CENTER, 0, 0);

	/* 有消息弹窗时, 除了弹窗外,其他区域变为深色, 有那个 聚焦的效果 */
	lv_obj_set_style_local_bg_opa(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_70);
	lv_obj_set_style_local_bg_color(cont, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x000000));

	lv_obj_t *obj = lv_cont_create(cont, NULL);
	lv_obj_set_size(obj, 600, 360);
	lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_auto_realign(obj, true);

	if (str != NULL)
	{
		lv_obj_set_style_local_value_str(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, str);
	}
	lv_obj_set_style_local_value_font(obj, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, FONT_SIZE_L(31));
	lv_obj_set_style_local_value_align(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_ALIGN_CENTER);
	lv_obj_set_style_local_bg_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
	lv_obj_set_style_local_bg_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x1C1C1C));

	/* 灰色边框 */
	lv_obj_set_style_local_border_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
	lv_obj_set_style_local_border_color(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x5C5C5C));
	lv_obj_set_style_local_border_width(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 1);
	lv_obj_set_style_local_border_side(obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_BORDER_SIDE_FULL);
	lv_obj_set_style_local_border_opa(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, LV_OPA_100);
	lv_obj_set_style_local_border_color(obj, LV_OBJ_PART_MAIN, LV_STATE_FOCUSED, lv_color_hex(0x5C5C5C));

	if (is_loading)
	{
		lv_obj_t *preload = lv_spinner_create(obj, NULL);
		lv_obj_set_size(preload, 120, 120);
		lv_obj_align(preload, preload->parent, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_style_local_line_color(preload, LV_SPINNER_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0X2183EF));

		lv_obj_set_style_local_line_color(preload, LV_SPINNER_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0XFFFFFF));
		lv_obj_set_style_local_line_width(preload, LV_SPINNER_PART_INDIC, LV_STATE_DEFAULT, 12);
		lv_obj_set_style_local_line_width(preload, LV_SPINNER_PART_BG, LV_STATE_DEFAULT, 12);

		/* 如果有加载框， 需要把文字向下偏移 */
		lv_obj_set_style_local_value_ofs_y(obj, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 100);
	}

	lv_obj_set_event_cb(cont, msg_window_event_cb);
	return cont;
}

static void msgbox_event_cb(lv_obj_t *obj, lv_event_t event)
{
	// Debug("=========================>>>>%p %p\n\n",obj,obj->user_data);
	lv_obj_t *group = (lv_obj_t *)(obj->user_data);
	if (event == LV_EVENT_DELETE)
	{
		// 启用按键操作
		// Debug("=========================>>>>%p %p\n\n",obj,obj->user_data);
		if (group)
			lv_obj_del(group);
	}
}

lv_obj_t *msgbox_animat_create(char *str, int ms)
{
	lv_obj_t *msg = lv_msgbox_create(lv_scr_act(), NULL);

	lv_obj_set_pos(msg, 240, 145);
	lv_obj_set_size(msg, 550, 300);

	lv_msgbox_set_text(msg, str);
	lv_obj_set_style_local_bg_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x1A1A1A));
	lv_obj_set_style_local_bg_opa(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, LV_OPA_COVER);

	lv_obj_set_style_local_border_width(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, 2);
	lv_obj_set_style_local_border_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xEFCC8C));
	lv_obj_set_style_local_text_font(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, FONT_SIZE(28));
	lv_obj_set_style_local_text_color(msg, LV_MSGBOX_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xffffff));
	lv_msgbox_start_auto_close(msg, ms);

	lv_obj_t *group = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_pos(group, 0, 0);
	lv_obj_set_size(group, 1024, 600);

	msg->user_data = group;
	lv_obj_set_event_cb(msg, msgbox_event_cb);
	// Debug("=========================>>>>%p %p\n\n",msg,group);
	return msg;
}

void network_devices_enable_init(void)
{
	extern void device_enable_state_set(network_device device, bool *enable);
	device_enable_state_set(DEVICE_OUTDOOR_1, &(user_data_get()->door1.enable_sw));
	device_enable_state_set(DEVICE_OUTDOOR_2, &(user_data_get()->door2.enable_sw));
	device_enable_state_set(DEVICE_CCTV_1, &(user_data_get()->camera1.enable));
	device_enable_state_set(DEVICE_CCTV_2, &(user_data_get()->camera2.enable));
	device_cctv_url_set(DEVICE_CCTV_1, user_data_get()->camera1.url);
	device_cctv_url_set(DEVICE_CCTV_2, user_data_get()->camera2.url);
}

int tuya_channel_valid_report_impl(void)
{
	tuya_ch_info_t info[MON_CH_TOTAL];
	int ch = monitor_channel_get();
	int count = 0;
	memset(info, 0, sizeof(info));

	for(int i = MON_CH_DOOR_1; i < MON_CH_TOTAL; i++)
	{
		if(monitor_valid_channel_check(i))
		{
			info[count].ch = i;
			info[count].name = (ch == i ? text_str(STR_TUYA_CURR_DOOR1 + (i - MON_CH_DOOR_1) * 2) : text_str(STR_TUYA_CURR_DOOR1 + (i - MON_CH_DOOR_1) * 2 + 1));
			count++;
		}
	}
	return tuya_channel_report(ch, info, count);
}
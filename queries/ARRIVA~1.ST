

!Arrival911Window publicMethods !

Minutes
|currow value rows|

value := String new.
currow := String new.

((self subpartNamed: 'resultTable of Minutes Grabber') valueOfAttributeNamed: #rows selector: #'IS_rows')
	do: [:curritem |
	curritem isNil ifFalse: [
	currow := (curritem at: 'MIN_COL') trimBlanks.
	value := ((value, ','), (currow)) trimBlanks.
	].].

value at: 1 put: $ .
value := (value) trimBlanks.

(self subpartNamed: 'Text3') object: value.!

MinuteUpdater
|ListString CurrChar ListColl each newChar il stringSize where|

ListString := String new.
CurrChar := String new: 3.
newChar := String new.
ListColl := OrderedCollection new.
ListString := (self subpartNamed: 'Text3') object.
each := 1.
il := 1.
stringSize := ListString size.

where := 'values '.
stringSize timesRepeat: [
	newChar := ListString at: each.

	newChar = $, 
	ifTrue: [
	ListColl add: CurrChar.
	where := where, '(',CurrChar, ')', ','.
	CurrChar := String new: 3.
	il := 1.]
	ifFalse: [
	CurrChar  at: il put:newChar.
	il := il + 1].
	each := each + 1.
].
ListColl add: CurrChar.
where := where,'(',CurrChar,')'.

(self subpartNamed: 'Minute Clearer') executeQueryAsTransaction.
(self subpartNamed: 'Minute Update') abtAtAttribute: #whereClause put: where.
(self subpartNamed: 'Minute Update') executeQueryAsTransaction.

"Response Times"
(self subpartNamed: 'Packeting Container Details1') statusLabelString: 'data'.
(self subpartNamed: 'Packeting Container Details2') statusLabelString: 'reli'.
(self subpartNamed: 'Packeting Container Details4') statusLabelString: 'stats'.

"Scene Times"
(self subpartNamed: 'Packeting Container Details7') statusLabelString: 'data'.
(self subpartNamed: 'Packeting Container Details8') statusLabelString: 'reli'.
(self subpartNamed: 'Packeting Container Details9') statusLabelString: 'stats'.

"Total Time"
(self subpartNamed: 'Packeting Container Details13') statusLabelString: 'data'.
(self subpartNamed: 'Packeting Container Details12') statusLabelString: 'reli'.
(self subpartNamed: 'Packeting Container Details15') statusLabelString: 'stats'.

"Travel Time"
(self subpartNamed: 'Packeting Container Details14') statusLabelString: 'data'.
(self subpartNamed: 'Packeting Container Details11') statusLabelString: 'reli'.
(self subpartNamed: 'Packeting Container Details16') statusLabelString: 'stats'.
(self subpartNamed: 'Packeting Container Details10') statusLabelString: 'codes'.

"Average response by district"
(self subpartNamed: 'Packeting Container Details3') statusLabelString: 'data'.!

RemoveDumbie: containerName with: colName with: tabName with: qName

|curr_atr where atr_name str_dta wg apos|

apos := String new: 1.
str_dta := String new.
where := tabName,' set ', colName, ' = null where '.
apos at:1 put:$'.

(self subpartNamed: containerName) selectedItems 
do: 	
[:	current_item| 
	where := where, '('.
	(self subpartNamed:containerName) columns do:
	[: curr2|
		atr_name :=curr2 attributeName.
		curr_atr := (current_item abtAtAttribute:atr_name).
		(curr_atr class = Time) ifTrue:[str_dta := ' = ', ((curr_atr asSeconds) printString).].
		(curr_atr class = SmallInteger) ifTrue:[str_dta := ' = ',curr_atr printString.].
		(curr_atr class = Integer) ifTrue:[str_dta := ' = ',curr_atr printString.].
		(curr_atr isNil) ifTrue:[str_dta := ' is null '.].
		(curr_atr class = String) ifTrue:[str_dta := ' = ', apos, curr_atr, apos].
		where := where, atr_name asString, str_dta, ' and '.
	].
	where at:((where size)-1) put:$ .
	where at:((where size)-2) put:$ .
	where at:((where size)-3) put:$ .
	where trimBlanks.
	where := where, ') or '.
].
where at:((where size)-1) put:$ .
where at:((where size)-2) put:$ .
where trimBlanks.

(self subpartNamed: 'Updater') abtAtAttribute: #whereClause put: where.
(self subpartNamed: 'Updater') executeQueryAsTransaction.

"If the redisplaying query has a whereClause, put nothing in it"
(self subpartNamed: qName) abtAtAttribute: #whereClause ifAbsent:
	[
	(self subpartNamed: qName) executeQueryAsTransaction.
	^self.
	].
"otherwise just do it"
(self subpartNamed: qName) abtAtAttribute: #whereClause put: ' '.
(self subpartNamed: qName) executeQueryAsTransaction.
!

saver: windowID with:fileName

|test atr_name curr_atr str_dta file path filepath fileStream counter|

counter := 1.
path := (self subpartNamed: 'Text4') object.
path == nil
ifFalse: 
	[
"loop through all Packeting Container Details"
	"loop through all Container Detail Columns"
		"output to file"
	(self subpartNamed: windowID)  components do:
		[: current_item|
		(current_item class = AbtPacketingContainerDetailsView) ifTrue:
			[
			self halt.
			file := '\', fileName, (current_item statusLabelString), '.txt'.
			filepath := path, file.
			fileStream := CfsWriteFileStream open: filepath.
			(current_item )columns do:
				[ :curr3|
				fileStream nextPutAll: curr3 heading.
				fileStream tab.
				].
			fileStream cr.
			((current_item items) rows) do: 
			[: curr1|
			(current_item columns) do:
				[: curr2|
				atr_name :=curr2 attributeName.
				curr_atr := (curr1 abtAtAttribute:atr_name).
				(curr_atr class = Time) ifTrue:[str_dta := ((self subpartNamed: 'Apc Stat Functions1') timeFormat: (curr_atr))].
				(curr_atr class = SmallInteger) ifTrue:[str_dta := curr_atr printString.].
				(curr_atr class = Integer) ifTrue:[str_dta := curr_atr printString.].
				(curr_atr class = ScaledDecimal) ifTrue:[str_dta := ((curr_atr asFloat) printString).].
				(curr_atr class = Float) ifTrue:[str_dta := curr_atr printString.].
				(curr_atr class = String) ifTrue:[str_dta := curr_atr.].
				(curr_atr isNil) ifTrue:[str_dta := ''.].
				fileStream nextPutAll: str_dta.
				fileStream tab.
				].
			fileStream cr.
			].
		fileStream close.
		counter := counter + 1.
		].
	].
].
!

saveResp
| where currtime ssize counter|
counter := 0.

where := 'values '.
(self subpartNamed: 'Packeting Container Details1')items rows do:
	[ :current_item| 	
	where := where, '(', ((current_item abtAtAttribute:#INCIDENT_CD) printString),','.

	currtime := (current_item abtAtAttribute:#ILSTIME).
	currtime isNil  ifTrue: [ where := where, 'null,'. ]
								ifFalse: [	where := where, (currtime asSeconds) printString,','.].

	currtime := (current_item abtAtAttribute:#ALSTIME).
	currtime isNil ifTrue: [ where := where, 'null,'.]
								ifFalse: [ where := where,(currtime asSeconds) printString,','.].

	currtime := (current_item abtAtAttribute:#FRTIME).
	currtime isNil ifTrue: [ where := where, 'null),'.]
								ifFalse: [ where := where,(currtime asSeconds) printString, '),'.].

	counter := counter + 1.
	((counter rem: 200) == 0) ifTrue:
		[
		ssize := where size.
		where at: ssize put:$ .
		where trimBlanks.
		(self subpartNamed: 'Create ArrvTemp') abtAtAttribute: #whereClause put: where.
		(self subpartNamed: 'Create ArrvTemp') executeQueryAsTransaction.
		where := 'values '.
		].
	].
((counter rem: 200) == 0) ifFalse:
	[
	ssize := where size.
	where at: ssize  put:$ .
	where trimBlanks.
	(self subpartNamed: 'Create ArrvTemp') abtAtAttribute: #whereClause put: where.
	(self subpartNamed: 'Create ArrvTemp') executeQueryAsTransaction.
	(self subpartNamed: 'Arrival9111Reliability') executeQueryAsTransaction.
	].!

SaveScene
| where currtime ssize counter|
counter := 0.

where := 'values '.
(self subpartNamed: 'Packeting Container Details7')items rows do:
	[ :current_item| 	
	where := where, '(', ((current_item abtAtAttribute:#INCIDENT_CD) printString),','.

	currtime := (current_item abtAtAttribute:#ALS_SCENE_TIME).
	currtime isNil  ifTrue: [ where := where, 'null,'. ]
								ifFalse: [	where := where, (currtime asSeconds) printString,','.].

	currtime := (current_item abtAtAttribute:#ILS_SCENE_TIME).
	currtime isNil ifTrue: [ where := where, 'null), '.]
								ifFalse: [ where := where,(currtime asSeconds) printString,'), '.].
	counter := counter + 1.
	((counter rem: 200) == 0) ifTrue:
		[
		ssize := where size.
		where at:(ssize -1)  put:$ .
		(self subpartNamed: 'Create scntmtmp') abtAtAttribute: #whereClause put: where.
		(self subpartNamed: 'Create scntmtmp') executeQueryAsTransaction.
		where := 'values '.
		].
	].
((counter rem: 200) == 0) ifFalse:
	[
	self halt.
	ssize := where size.
	where at:(ssize-1)  put:$ .
	where trimBlanks.
	(self subpartNamed: 'Create scntmtmp') abtAtAttribute: #whereClause put: where.
	(self subpartNamed: 'Create scntmtmp') executeQueryAsTransaction.
	(self subpartNamed: 'Scene Time Reliability') executeQueryAsTransaction.
	].
!

selectiondelete
|diag_cd items|

(self subpartNamed: 'Packeting Container Details5') selectedItems do: [:current_item|
	diag_cd := current_item abtAtAttribute: #DIAG_CD.
	(self subpartNamed: 'Delete Diag Codes') abtAtAttribute: 'HV_DIAG_CD' put: 	diag_cd .
	(self subpartNamed: 'Delete Diag Codes') executeQueryAsTransaction.
]!

selectioninsert
|diag_cd items|

(self subpartNamed: 'Packeting Container Details6') selectedItems do: [:current_item|
	diag_cd := current_item abtAtAttribute: #DIAG_CD.
	(self subpartNamed: 'Insert Diag Code') abtAtAttribute: 'HV_DIAG_CD' put: 	diag_cd .
	(self subpartNamed: 'Insert Diag Code') executeQueryAsTransaction.
]!

SortBy: column with: qname
| wg |
wg := 'order by ',column,' desc'.
(self subpartNamed: qname) abtAtAttribute: #whereClause put: wg.
(self subpartNamed: qname) executeQueryAsTransaction.! !
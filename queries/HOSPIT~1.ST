

!HospitalInfo publicMethods !

DynamicDxCds
|diag_cd items|

(self subpartNamed: 'Packeting Container Details1') selectedItems do: [:current_item|
	diag_cd := current_item abtAtAttribute: #DIAG_CD.
	(self subpartNamed: 'Delete Diag Codes') abtAtAttribute: 'HV_DIAG_CD' put: 	diag_cd .
	(self subpartNamed: 'Delete Diag Codes') executeQueryAsTransaction.
]!

InsertSelection
|diag_cd items|

(self subpartNamed: 'Packeting Container Details2') selectedItems do: [:current_item|
	diag_cd := current_item abtAtAttribute: #DIAG_CD.
	(self subpartNamed: 'Insert Diag Code') abtAtAttribute: 'HV_DIAG_CD' put: 	diag_cd .
	(self subpartNamed: 'Insert Diag Code') executeQueryAsTransaction.
]!

Minutes
|currow value rows|

value := String new.
currow := String new.

((self subpartNamed: 'resultTable of Minutes GrabberHS') valueOfAttributeNamed: #rows selector: #'IS_rows')
	do: [:curritem |
	curritem isNil ifFalse: [
	currow := (curritem at: 'MIN_COL') trimBlanks.
	value := ((value, ','), (currow)) trimBlanks.
	].].

value at: 1 put: $ .
value := (value) trimBlanks.

(self subpartNamed: 'Text3') object: value.!

MinuteUpdater
|ListString CurrChar ListColl each newChar il stringSize|

ListString := String new.
CurrChar := String new: 3.
newChar := String new.
ListColl := OrderedCollection new.
ListString := (self subpartNamed: 'Text3') object.
each := 1.
il := 1.
stringSize := ListString size.

stringSize timesRepeat: [
	newChar := ListString at: each.

	newChar = $, 
	ifTrue: [
	ListColl add: CurrChar.
	CurrChar := String new: 3.
	il := 1.]
	ifFalse: [
	CurrChar  at: il put:newChar.
	il := il + 1].
	each := each + 1.
].
ListColl add: CurrChar.

(self subpartNamed: 'Minute Clearer') executeQueryAsTransaction.
ListColl do: [:current_item|
	CurrChar := current_item.
	(self subpartNamed: 'Minute Update') abtAtAttribute: 'HV_MINUTE' put:CurrChar.
	(self subpartNamed: 'Minute Update') executeQueryAsTransaction.
].
!

RemoveDumbie: containerName with: colName with: tabName with: qName

|curr_atr where atr_name str_dta|

str_dta := String new.
where := tabName,' set ', colName, ' = null where '.

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
		(curr_atr isNil) ifTrue:[str_dta = ' is nil '.].
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
self halt.
(self subpartNamed: 'Updater') abtAtAttribute: #whereClause put: where.
(self subpartNamed: 'Updater') executeQueryAsTransaction.

(self subpartNamed: qName) executeQueryAsTransaction.!

whereWriter: InOut
|where oper apos startDate endDate|

apos := String new: 1.
apos at:1 put:$'.
startDate := String new.
endDate := String new.

startDate :=(self subpartNamed: 'Text1') object.
startDate := 'date(',apos,(startDate monthIndex) printString, '/',(startDate dayOfMonth) printString , '/', (startDate year) printString,apos,')'.
endDate :=(self subpartNamed: 'Text2') object.
endDate := 'date(',apos,(endDate monthIndex) printString, '/',(endDate dayOfMonth) printString , '/', (endDate year) printString,apos,')'.

(InOut = 'in') ifTrue:
	[
	oper := ' = '.
	]
ifFalse:
	[
	oper := ' <> '.
	].

where := ' where ('.

(self subpartNamed: 'Multiple Select List1') selectionIndices do: 
	[:current_item|
	where := where, 'i.loc_hosp_catch_cd ', oper,  current_item printString, ' or '.
	].
where at:((where size)-1) put:$ .
where at:((where size)-2) put:$ .
where at:((where size)-3) put:$).
where trimBlanks.
where := where, 'and i.create_date between ',startDate,' and ', endDate, ' and ('.

(self subpartNamed: 'Multiple Select List2') selectedItems do:
	[:current_item|
	where := where, 'i.apctransport = ',apos,(current_item at:1) asString,apos, ' or '.
	].
where at:((where size)-1) put:$ .
where at:((where size)-2) put:$ .
where at:((where size)-3) put:$).
where trimBlanks.
where := where, 	' and P.DIAG_CD IN D.DIAG_CD	AND
	I.INCIDENT_CD = P.INCIDENT_CD AND
	U.INCIDENT_CD = I.INCIDENT_CD'.
self halt.
(self subpartNamed: 'In Catchment Insert') abtAtAttribute: #whereClause put: where.
(self subpartNamed: 'Out Catchment insert') abtAtAttribute: #whereClause put: where.




! !
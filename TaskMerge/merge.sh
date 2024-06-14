
merge() {
	python3 pngs2pdf_phi_${1}.py ~/work/alice/downloads/${2}/${3}/${1} ./output/${1}_${2}_${3}.pdf
}

merge negEta PbPb2023_MinusA11_apass3_20240515 lowIR

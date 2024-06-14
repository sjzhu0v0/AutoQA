
merge() {
	python3 pngs2pdf_phi.py ~/work/alice/downloads/${2}/${3}/${1} ./output/${1}_${2}_${3}.pdf
}

for i in lowIR  zzk  zzl  zzm  zzn  zzo
do
merge negEta PbPb2023_MinusA11_apass3_20240515 ${i}
merge posEta PbPb2023_MinusA11_apass3_20240515 ${i}
done

merge posEta TPCTrees_pp2024_cpass0_20240510 af_sector11
merge negEta TPCTrees_pp2024_cpass0_20240510 af_sector11

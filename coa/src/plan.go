package main

import (
	"fmt"
	"os"
)

// Action rappresenta un singolo blocco "command" nell'array "plan"
type Action struct {
	Command         string `json:"command"`
	VolID           string `json:"volid,omitempty"`
	OutputISO       string `json:"output_iso,omitempty"`
	CryptedPassword string `json:"crypted_password,omitempty"`
}

// FlightPlan rappresenta l'intero piano da passare a oa
type FlightPlan struct {
	PathLiveFs      string   `json:"pathLiveFs"`
	Mode            string   `json:"mode"`
	InitrdCmd       string   `json:"initrd_cmd"`
	BootloadersPath string   `json:"bootloaders_path"`
	Plan            []Action `json:"plan"`
}


func GeneratePlan(d *Distro, mode string, workPath string) FlightPlan {
	plan := FlightPlan{
		PathLiveFs: workPath,
		Mode:       mode,
	}

	// Gestione Initrd basata sulla famiglia [cite: 28]
	// ... (switch d.FamilyID esistente) ...

	// Gestione dinamica dei Bootloader per Arch/Fedora [cite: 28]
	if d.FamilyID != "debian" {
		btPath, err := EnsureBootloaders()
		if err != nil {
			fmt.Printf("\033[1;31m[coa]\033[0m Errore critico bootloaders: %v\n", err)
			os.Exit(1)
		}
		plan.BootloadersPath = btPath
	} else {
		plan.BootloadersPath = "" // Su Debian usa quelli di sistema
	}

	// 3. Assemblaggio dinamico della catena di montaggio
	plan.Plan = []Action{
		{Command: "action_prepare"},
		{Command: "action_users"}, // oa sa già come gestirlo in base al "mode"
		{Command: "action_initrd"},
		{Command: "action_livestruct"},
		{Command: "action_isolinux"},
		{Command: "action_uefi"},
		{Command: "action_squash"},
	}

	// Inserzione modulare: se l'utente vuole l'ISO cifrata, aggiungiamo l'azione in mezzo!
	if mode == "crypted" {
		plan.Plan = append(plan.Plan, Action{
			Command:         "action_crypted",
			CryptedPassword: "evolution", // Qui potremmo passarla da linea di comando in futuro
		})
	}

	// 4. Generazione ISO e chiusura
	// Costruiamo un nome file parlante e dinamico!
	isoName := fmt.Sprintf("egg-of_%s-%s-oa_amd64.iso", d.DistroID, d.CodenameID)
	
	plan.Plan = append(plan.Plan, Action{
		Command:   "action_iso",
		VolID:     "OA_LIVE",
		OutputISO: isoName,
	})
	
	plan.Plan = append(plan.Plan, Action{Command: "action_cleanup"})

	return plan
}
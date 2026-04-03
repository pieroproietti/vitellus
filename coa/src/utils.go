package main

import (
	"archive/tar"
	"compress/gzip"
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
)

const (
	BootloaderURL  = "https://github.com/pieroproietti/penguins-bootloaders/releases/download/v26.1.16/bootloaders.tar.gz"
	BootloaderRoot = "/tmp/coa"
)

// EnsureBootloaders verifica la presenza dei bootloader e li scarica se mancano
func EnsureBootloaders() (string, error) {
	targetDir := filepath.Join(BootloaderRoot, "bootloaders")

	// 1. Controllo se esistono già
	if _, err := os.Stat(targetDir); err == nil {
		return targetDir, nil
	}

	fmt.Printf("\033[1;33m[coa]\033[0m Bootloaders non trovati. Inizio download...\n")
	
	// 2. Download
	resp, err := http.Get(BootloaderURL)
	if err != nil {
		return "", err
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return "", fmt.Errorf("errore download: status %d", resp.StatusCode)
	}

	// 3. Estrazione on-the-fly
	if err := extractTarGz(resp.Body, BootloaderRoot); err != nil {
		return "", err
	}

	return targetDir, nil
}

func extractTarGz(r io.Reader, dest string) error {
	gzr, err := gzip.NewReader(r)
	if err != nil {
		return err
	}
	defer gzr.Close()

	tr := tar.NewReader(gzr)
	for {
		header, err := tr.Next()
		if err == io.EOF {
			break
		}
		if err != nil {
			return err
		}

		target := filepath.Join(dest, header.Name)
		switch header.Typeflag {
		case tar.TypeDir:
			if err := os.MkdirAll(target, 0755); err != nil {
				return err
			}
		case tar.TypeReg:
			f, err := os.OpenFile(target, os.O_CREATE|os.O_RDWR, os.FileMode(header.Mode))
			if err != nil {
				return err
			}
			if _, err := io.Copy(f, tr); err != nil {
				f.Close()
				return err
			}
			f.Close()
		}
	}
	return nil
}

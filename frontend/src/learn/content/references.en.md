# References

This page lists, by topic, the textbooks and papers cited in the primer chapters, each with a one-line note on what to read it for. Bibliographic entries give authors, title, publisher or journal, volume, page and year; DOIs are not listed — search the publisher's site by title when needed. The chapters follow AVAS's actual behaviour; where it differs from textbook conventions, the "AVAS convention" callouts of each chapter say so.

## Textbooks {#references-textbooks}

- T. P. Wangler, *RF Linear Accelerators*, 2nd ed., Wiley-VCH, Weinheim, 2008. — The standard text on RF linacs: cavities and transit-time factors, longitudinal and transverse dynamics, space charge, the mismatch factor, halo, superconducting linacs; the primer follows its notation.
- M. Reiser, *Theory and Design of Charged Particle Beams*, 2nd ed., Wiley-VCH, Weinheim, 2008. — A systematic treatment of charged-particle beams: periodic focusing, KV and rms envelope equations, space-charge-dominated beams, equipartitioning, free energy of mismatch and emittance growth.
- S. Y. Lee, *Accelerator Physics*, World Scientific (several editions). — A comprehensive textbook with clear treatment of transverse linear optics, Courant–Snyder theory, orbit distortion and correction, and many exercises.
- H. Wiedemann, *Particle Accelerator Physics*, 4th ed., Springer, 2015. — A comprehensive reference (the 4th edition is open access) with systematic derivations of beam optics, phase space, magnets and errors.
- A. W. Chao, K. H. Mess, M. Tigner, F. Zimmermann (eds.), *Handbook of Accelerator Physics and Engineering*, 2nd ed., World Scientific, 2013. — Handbook of formulas and data, for looking up common formulas and typical parameters.
- S. Humphries, *Charged Particle Beams*, Wiley, 1990. — An introduction to beam transport, focusing elements and space charge, well suited to self-study by graduate students.
- S. Humphries, *Principles of Charged Particle Acceleration*, Wiley, 1986. — An introduction to the principles of accelerators, including electrostatic and RF acceleration and magnetic focusing.

## Linacs, RF and superconducting cavities {#references-linac}

- P. M. Lapostolle, A. L. Septier (eds.), *Linear Accelerators*, North-Holland, Amsterdam, 1970. — The classic collection on linacs, an original source of early design methods and beam dynamics.
- I. M. Kapchinsky, *Theory of Resonance Linear Accelerators*, Harwood Academic, 1985. — A systematic theory of longitudinal and transverse motion in resonance linacs.
- H. Padamsee, J. Knobloch, T. Hays, *RF Superconductivity for Accelerators*, Wiley, 1998 (2nd ed. Wiley-VCH, 2008). — Physics and technology of superconducting cavities: surface resistance, cavity shapes, gradient limits; read it to understand the design constraints of HWR, spoke and elliptical cavities.

## Classic papers on linear optics and phase space {#references-classics}

- H. Busch, "Berechnung der Bahn von Kathodenstrahlen im axialsymmetrischen elektromagnetischen Felde", *Ann. Phys.* 386, 974 (1926). — Busch's theorem: conservation of canonical angular momentum in axisymmetric magnetic fields, the starting point of solenoid focusing and Larmor rotation.
- E. D. Courant, H. S. Snyder, "Theory of the alternating-gradient synchrotron", *Ann. Phys.* 3, 1 (1958). — The founding paper of strong focusing: stability of periodic focusing, β functions and Twiss parameters, phase advance.

## Space charge, envelopes, matching and halo {#references-space-charge}

- I. M. Kapchinskij, V. V. Vladimirskij, "Limitations of proton beam current in a strong focusing linear accelerator associated with the beam space charge", in *Proc. Int. Conf. on High Energy Accelerators and Instrumentation*, CERN, Geneva, 1959, p. 274. — The original paper of the KV distribution and the KV envelope equations.
- F. J. Sacherer, "RMS envelope equations with space charge", *IEEE Trans. Nucl. Sci.* NS-18, 1105 (1971). — Shows that the rms envelope equations hold for any elliptically symmetric distribution; the theoretical basis of rms matching and rms-equivalent beams.
- P. M. Lapostolle, "Possible emittance increase through filamentation due to space charge in continuous beams", *IEEE Trans. Nucl. Sci.* NS-18, 1101 (1971). — Definition of the rms emittance and emittance growth through space-charge filamentation.
- I. Hofmann, L. J. Laslett, L. Smith, I. Haber, "Stability of the Kapchinskij-Vladimirskij (K-V) distribution in long periodic transport systems", *Part. Accel.* 13, 145 (1983). — Instabilities of KV beams in periodic channels, one theoretical source of the phase-advance rules.
- J. Struckmeier, M. Reiser, "Theoretical studies of envelope oscillations and instabilities of mismatched intense charged-particle beams in periodic focusing channels", *Part. Accel.* 14, 227 (1984). — Envelope oscillation modes and the envelope instability, the origin of the $\sigma_0 < 90^\circ$ rule.
- I. Hofmann, "Stability of anisotropic beams with space charge", *Phys. Rev. E* 57, 4713 (1998). — Space-charge coupling resonances of anisotropic beams and the stability charts (Hofmann charts).
- I. Hofmann, J. Qiang, R. D. Ryne, "Collective resonance model of energy exchange in 3D nonequipartitioned beams", *Phys. Rev. Lett.* 86, 2313 (2001). — Emittance exchange in 3D non-equipartitioned beams and its resonance model; read it to understand when the equipartitioning rule is needed.
- R. L. Gluckstern, "Analytic model for halo formation in high current ion linacs", *Phys. Rev. Lett.* 73, 1247 (1994). — The analytic model explaining halo by the particle–core resonance.
- T. P. Wangler, K. R. Crandall, R. Ryne, T. S. Wang, "Particle-core model for transverse dynamics of beam halo", *Phys. Rev. ST Accel. Beams* 1, 084201 (1998). — A systematic study of the particle–core model and the relation between mismatch and maximum halo amplitude.
- K. R. Crandall, D. P. Rusthoi, *TRACE 3-D Documentation*, 3rd ed., Los Alamos report LA-UR-97-886, 1997. — Manual of the classic 3D linear envelope code, with its conventions for the mismatch factor, matching algorithms and element matrices; useful for comparison with AVAS's linear preview.

## Errors and correction {#references-errors}

- T. O. Raubenheimer, R. D. Ruth, "A dispersion-free trajectory correction technique for linear colliders", *Nucl. Instrum. Methods A* 302, 191 (1991). — A representative paper on trajectory correction in linacs (for electron linear colliders); read it for the basic ideas of correction algorithms.
- Linear error propagation, tolerances and one-to-one orbit correction: see the corresponding chapters of Wangler and Lee, and the estimates in the [errors and correction](#errors) chapter.

## AVAS {#references-avas}

- C. Jin, Z.-J. Wang, X. Qi, Y. He, K. Li, et al., Phys. Rev. Accel. Beams 28, 044602 (2025) — the AVAS reference (listed in the "About AVAS" dialog); please cite it when publishing results obtained with AVAS.
- AVAS user manual `docs/使用说明20260427.docx` — the physical original reference for keywords, parameters, units and file formats; parameter meanings follow it and are collected in the in-app "Parameters and files" reference and in `avas/data/schema.py`.
- The in-app user manual (Help menu; "Operation guide / Case tutorials / Parameters and files") — GUI operation, verified cases and their run conditions.
- The [first chapter](#intro) of this primer introduces the parts of AVAS and the reading order of the primer.

## Lecture notes and online resources {#references-online}

- CERN Accelerator School (CAS) proceedings, [https://cas.web.cern.ch](https://cas.web.cern.ch) — lecture notes on introductory and intermediate accelerator physics, RF, intense beams and other topics, freely available.
- US Particle Accelerator School (USPAS) course materials, [https://uspas.fnal.gov](https://uspas.fnal.gov) — lecture notes and exercises from courses on linacs, beam physics and more.
- JACoW conference proceedings, [https://www.jacow.org](https://www.jacow.org) — full texts of LINAC, IPAC, HB and other conference papers, for design and error-study examples of specific accelerators.

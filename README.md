<p align="center">
  <img src="se_logo.png"/>
</p>

----

# Yocto Layer for System Electronics SoM 'Astrial'

This Yocto Project / OpenEmbedded layer **meta-sysele-nxp-6.6.52** provides
the configuration and recipes used to build Linux evaluation images for
the **System Electronics Astrial** system on module.

This repository targets **Yocto Project 5.0 (Scarthgap)**.
For an overview of the available Astrial BSP releases, documentation and
prebuilt image downloads, see the
[Astrial getting started guide](https://github.com/System-Electronics/astrial-howto).

# Dependencies

The setup is aligned with **NXP Linux BSP `imx-6.6.52-2.2.0`**.

Use the manifests and setup instructions provided by this layer to select
the required repositories and revisions.

# Building

See [ASTRIAL-YOCTO-INSTALL.md](ASTRIAL-YOCTO-INSTALL.md) for environment setup,
image build instructions and board programming.

The guide covers the Astrial **2 GB, 4 GB and 8 GB RAM** configurations.

# Patches

This layer is maintained by System Electronics.

When creating a patch of the last commit, use:

```sh
git format-patch -s --subject-prefix='meta-sysele][<branch>][PATCH' -1
```

To send patches, use:

```sh
git send-email --to github@systemelectronics.com <generated-patch>
```

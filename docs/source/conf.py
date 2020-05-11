# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

import os
from pathlib import Path
import pkg_resources
import shutil
import subprocess

from jinja2 import Template

project = "libsonata"
author = "Blue Brain Project, EPFL"
version = pkg_resources.get_distribution("libsonata").version
release = version

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ["sphinx.ext.autodoc", "sphinx.ext.napoleon"]

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

master_doc = "index"

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
html_theme = "sphinx-bluebrain-theme"
html_title = "libsonata"
html_show_sourcelink = False
html_extra_path = ["doxygen"]
html_theme_options = {
    "repo_url": "https://github.com/BlueBrain/libsonata/",
    "repo_name": "BlueBrain/libsonata",
}

autodoc_default_options = {"members": True, "imported-members": True}
autodoc_docstring_signature = True

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
# html_static_path = ['_static']


def doxygen_build(app):
    """Build the doxygen output for inclusion in the sphinx docs."""
    subprocess.call(["doxygen", "Doxyfile"], cwd=app.srcdir)


def render_doxygen_pages(app):
    """Render the templated output."""
    NAMESPACE_SEP = "_1_1"
    NAMESPACE_ROOT = "bbp"

    root_dir = Path(app.confdir)
    template_dir = root_dir / "_templates"
    output_path = root_dir / "cpp"
    doxygen_path = root_dir / "doxygen" / "cpp"

    if output_path.exists():
        shutil.rmtree(output_path)

    output_path.mkdir()

    template_file = template_dir / "doxygen_page.rst_t"
    page_template = Template(template_file.read_text())

    for f in doxygen_path.glob("*.html"):
        if f.name.endswith("-members.html"):
            continue

        if NAMESPACE_SEP not in f.name or NAMESPACE_ROOT not in f.name:
            continue

        type_, name = f.name.split(NAMESPACE_ROOT, maxsplit=1)
        name = NAMESPACE_ROOT + name

        name = "".join(
            s.capitalize() for s in name.replace(NAMESPACE_SEP, "::").split("_")
        ).rstrip(".html")
        name = name[0].lower() + name[1:]
        name = name.replace("bbp::sonata::", "")

        filename = "doxygen_" + name
        context = {"title": name, "file": f.name}

        output_file = output_path / (filename + ".rst")
        output_file.write_text(page_template.render(**context))


def setup(app):
    app.connect("builder-inited", doxygen_build)
    app.connect("builder-inited", render_doxygen_pages)

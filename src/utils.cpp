/*************************************************************************
 * Copyright (C) 2018-2020 Blue Brain Project
 *
 * This file is part of 'libsonata', distributed under the terms
 * of the GNU Lesser General Public License version 3.
 *
 * See top-level COPYING.LESSER and COPYING files for details.
 *************************************************************************/

#include "utils.h"

#include "../extlib/filesystem.hpp"

#include <fstream>

std::string readFile(const std::string& path) {
    namespace fs = ghc::filesystem;

    if (!fs::is_regular_file(path)) {
        throw std::runtime_error("Path `" + path + "` is not a file");
    }

    std::ifstream file(path);

    if (file.fail()) {
        throw std::runtime_error("Could not open file `" + path + "`");
    }

    std::string contents;

    file.seekg(0, std::ios::end);
    contents.reserve(file.tellg());
    file.seekg(0, std::ios::beg);

    contents.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    return contents;
}

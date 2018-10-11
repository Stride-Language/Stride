/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#ifndef STREAMPLATFORM_H
#define STREAMPLATFORM_H

#include <string>
#include <map>

#include <QList>

#include "ast.h"

class StridePlatform;

class StrideConnection {
public:

private:
    StridePlatform *m_source;
    StridePlatform *m_destination;
    AST *m_sourceImports;
    AST *m_sourceStreams;
    AST *m_destinationImports;
    AST *m_destinationStreams;
};

class StridePlatform {
public:
    StridePlatform(string framework, string fwVersion,
                   string hardware = "", string hardwareVersion = "",
                   string identifier = "") :
        m_framework(framework), m_frameworkVersion(fwVersion),
        m_hardware(hardware), m_hardwareVersion(hardwareVersion),
        m_identifier(identifier)
    {
    }

    ~StridePlatform();

    typedef enum {
        PythonTools,
        PluginPlatform,
        NullPlatform
    } PlatformAPI;

    string getFramework() const;
    string getFrameworkVersion() const;
    string getHardware() const;
    string getHardwareVersion() const;
    bool getRequired() const;
    PlatformAPI getAPI() const;
    string buildPlatformPath(string strideRoot);
    string buildLibPath(string strideRoot);
    string buildTestingLibPath(string strideRoot);

    void addTree(string treeName, ASTNode treeRoot);
    void addTestingTree(string treeName, ASTNode treeRoot);
    vector<ASTNode> getPlatformObjectsReference();
    vector<ASTNode> getPlatformTestingObjectsRef();

private:
    string m_framework;
    string m_frameworkVersion;
    string m_hardware;
    string m_hardwareVersion;
    string m_identifier;
    bool m_required;
    PlatformAPI m_api {PluginPlatform}; //TODO Put back support for plugin platforms
    map<string, ASTNode> m_platformTrees;
    map<string, ASTNode> m_platformTestTrees;
};



#endif // STREAMPLATFORM_H

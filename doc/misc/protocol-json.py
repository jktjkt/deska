import json

# Note: many JSON implementations would not respect the ordering of our keys,
# and in some of them, like Python's own json module, it would be hard to
# actually make it respect our order. Therefore, the Deska API doesn't mandate
# any particular order at each level.

C_PREFIX = "command"
R_PREFIX = "response"
IDENTIFIER = "identifier"
KIND_NAME = "kindName"
OBJ_NAME = "objectName"
ATTR_NAME = "attributeName"
REVISION = "revision"
E_PREFIX = "error"

cmd_kindNames = {C_PREFIX: "getTopLevelObjectNames"}
resp_kindNames = {R_PREFIX: "getTopLevelObjectNames",
                  "topLevelObjectKinds": ["z", "a", "b", "c"]}

cmd_kindAttributes = {C_PREFIX: "getKindAttributes", KIND_NAME: IDENTIFIER}
resp_kindAttributes = {R_PREFIX: "getKindAttributes", KIND_NAME: IDENTIFIER,
                       "kindAttributes": {"foo": "string",
                                          "bar": "int",
                                          "baz": "identifier",
                                          "price": "double"}}

cmd_kindRelations = {C_PREFIX: "getKindRelations", KIND_NAME: IDENTIFIER}
resp_kindRelations = {R_PREFIX: "getKindRelations", KIND_NAME: IDENTIFIER,
                      "kindRelations": [("EMBED_INTO", "hardware"),
                                        ("MERGE_WITH", "second-kind", "my-attribute"),
                                        ("IS_TEMPLATE", "target-kind"),
                                        ("TEMPLATIZED", "by-which-kind", "my-attribute")
                                       ]}

cmd_kindInstances = {C_PREFIX: "getKindInstances", KIND_NAME: IDENTIFIER,
                     REVISION: 0}
resp_kindInstances = {R_PREFIX: "getKindInstances", KIND_NAME: IDENTIFIER,
                      REVISION: 0, "objectInstances": ["foo", "bar", "ahoj"]}

cmd_objectData = {C_PREFIX: "getObjectData", KIND_NAME: IDENTIFIER,
                  OBJ_NAME: IDENTIFIER, REVISION: 0}
resp_objectData = {R_PREFIX: "getObjectData", KIND_NAME: IDENTIFIER,
                   OBJ_NAME: IDENTIFIER, REVISION: 0,
                   "objectData": {"foo": "bar", "price": 666} }

cmd_resolvedObjectData = {C_PREFIX: "getResolvedObjectData",
                          KIND_NAME: IDENTIFIER, OBJ_NAME: IDENTIFIER,
                          REVISION: 0}
resp_resolvedObjectData = {R_PREFIX: "getResolvedObjectData",
                           KIND_NAME: IDENTIFIER, OBJ_NAME: IDENTIFIER,
                           REVISION: 0,
                           "resolvedObjectData": {"foo": ("obj-defining-this",
                                                          "bar"),
                                                  "price": ("this-obj", 666)}
                          }

cmd_findOverridenAttrs = {C_PREFIX: "getObjectsOverridingAttribute",
                          KIND_NAME: IDENTIFIER, OBJ_NAME: IDENTIFIER,
                          ATTR_NAME: IDENTIFIER}
resp_findOverridenAttrs = {R_PREFIX: "getObjectsOverridingAttribute",
                           KIND_NAME: IDENTIFIER, OBJ_NAME: IDENTIFIER,
                           ATTR_NAME: IDENTIFIER,
                           "objectInstances": ["z", "a", "aaa"]}

cmd_findNonOverridenAttrs = {C_PREFIX: "getObjectsNotOverridingAttribute",
                             KIND_NAME: IDENTIFIER, OBJ_NAME: IDENTIFIER,
                             ATTR_NAME: IDENTIFIER}
resp_findNonOverridenAttrs = {R_PREFIX: "getObjectsNotOverridingAttribute",
                              KIND_NAME: IDENTIFIER, OBJ_NAME: IDENTIFIER,
                              ATTR_NAME: IDENTIFIER,
                              "objectInstances": ["d", "e", "aaaaa"]}

cmd_deleteObject = {C_PREFIX: "deleteObject", KIND_NAME: IDENTIFIER,
                    OBJ_NAME: IDENTIFIER}
resp_deleteObject = {R_PREFIX: "deleteObject", KIND_NAME: IDENTIFIER,
                     OBJ_NAME: IDENTIFIER, "result": True}

cmd_createObject = {C_PREFIX: "createObject", KIND_NAME: IDENTIFIER,
                    OBJ_NAME: IDENTIFIER}
resp_createObject = {R_PREFIX: "createObject", KIND_NAME: IDENTIFIER,
                     OBJ_NAME: IDENTIFIER, "result": True}

cmd_renameObject = {C_PREFIX: "renameObject", KIND_NAME: IDENTIFIER,
                    OBJ_NAME: IDENTIFIER, "newObjectName": IDENTIFIER}
resp_renameObject = {R_PREFIX: "renameObject", KIND_NAME: IDENTIFIER,
                     OBJ_NAME: IDENTIFIER, "newObjectName": IDENTIFIER,
                     "result": True}


cmd_removeAttribute = {C_PREFIX: "removeObjectAttribute", KIND_NAME: IDENTIFIER,
                       OBJ_NAME: IDENTIFIER, ATTR_NAME: IDENTIFIER}
resp_removeAttribute = {R_PREFIX: "removeObjectAttribute", KIND_NAME: IDENTIFIER,
                        OBJ_NAME: IDENTIFIER, ATTR_NAME: IDENTIFIER,
                        "result": True}

cmd_setAttribute = {C_PREFIX: "setObjectAttribute", KIND_NAME: IDENTIFIER,
                    OBJ_NAME: IDENTIFIER, ATTR_NAME: IDENTIFIER,
                    "attributeData": "value"}
resp_setAttribute = {R_PREFIX: "setObjectAttribute", KIND_NAME: IDENTIFIER,
                     OBJ_NAME: IDENTIFIER, ATTR_NAME: IDENTIFIER,
                     "attributeData": "value", "result": True}

cmd_startChangeset = {C_PREFIX: "vcsStartChangeSet"}
resp_startChangeset = {R_PREFIX: "vcsStartChangeSet", "revision": 123}

cmd_commit = {C_PREFIX: "vcsCommit"}
resp_commit = {R_PREFIX: "vcsCommit", "revision": 123}

cmd_rebaseTransaction = {C_PREFIX: "vcsRebaseTransaction", "currentRevision": 666}
resp_rebaseTransaction = {R_PREFIX: "vcsRebaseTransaction", "currentRevision": 666,
                          "revision": 333666}

cmd_pendingRevisionsByMyself = {C_PREFIX: "getPendingRevisionsByMyself"}
resp_pendingRevisionsByMyself = {R_PREFIX: "getPendingRevisionsByMyself",
                                 "revisions": []
                                }

cmd_resumePendingChangeset = {C_PREFIX: "resumePendingChangeset",
                              "currentRevision": 123}
resp_resumePendingChangeset = {R_PREFIX: "resumePendingChangeset",
                               "currentRevision": 123, "revision": 666}

cmd_abortChangeset = {C_PREFIX: "abortChangeset", "revision": 123}
resp_abortChangeset = {R_PREFIX: "abortChangeset", "revision": 123}

def exceptionify(item):
    """Each response can be extended to include a description of an error encountered
    during processing of the request/command/... at the remote side."""

    item[E_PREFIX] = ("IdentifierOfKindOfTheException", "Textual description",
                      ("optional", "list", "of", "arguments"))
    return item


for stuff in (cmd_kindNames, resp_kindNames, cmd_kindAttributes,
              resp_kindAttributes, cmd_kindRelations, resp_kindRelations,
              cmd_kindInstances, resp_kindInstances, cmd_objectData,
              resp_objectData, cmd_resolvedObjectData, resp_resolvedObjectData,
              cmd_findOverridenAttrs, resp_findOverridenAttrs,
              cmd_findNonOverridenAttrs, resp_findNonOverridenAttrs,
              cmd_deleteObject, resp_deleteObject, cmd_createObject,
              resp_createObject, cmd_renameObject, resp_renameObject,
              cmd_removeAttribute, resp_removeAttribute, cmd_setAttribute,
              resp_setAttribute, cmd_startChangeset, resp_startChangeset,
              cmd_commit, resp_commit, cmd_rebaseTransaction,
              exceptionify(resp_rebaseTransaction),
              cmd_pendingRevisionsByMyself, resp_pendingRevisionsByMyself,
              cmd_resumePendingChangeset, resp_resumePendingChangeset,
              cmd_abortChangeset, resp_abortChangeset
             ):
    print json.dumps(stuff, sort_keys=True)



CPMAddPackage(
    NAME gcem
    GITHUB_REPOSITORY kthohr/gcem
    VERSION 1.17.0
    OPTIONS
          "GCEM_BUILD_TESTS OFF"
)


set_target_properties(gcem PROPERTIES FOLDER "ThirdParty")

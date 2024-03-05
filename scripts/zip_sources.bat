pushd "%~dp0\.."
if EXIST "dist\MissionEditorSource.zip" (
  del dist\MissionEditorSource.zip
)
git archive -o "dist\MissionEditorSource.zip" HEAD
popd
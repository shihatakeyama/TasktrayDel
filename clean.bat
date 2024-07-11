@echo off

echo 削除します。宜しいですか？ 

del *.sdf /S /Q
del Debug /S /Q
del Release /S /Q
rmdir Debug /S /Q
rmdir Release /S /Q
del ipch /S /Q
rmdir ipch /S /Q
del TasktrayDel\Debug /S /Q
rmdir TasktrayDel\Debug /S /Q
del TasktrayDel\Release /S /Q
rmdir TasktrayDel\Release /S /Q



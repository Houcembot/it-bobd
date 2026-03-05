import json

block_content = {
    "object": "block",
    "type": "heading_3",
    "heading_3": {
      "rich_text": [{"text": {"content": "1. **Valeur Perçue :**"}}]
    }
  }
print(json.dumps({"children": [block_content]}))
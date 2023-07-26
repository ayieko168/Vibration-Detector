from requests_html import HTMLSession
import json
import re

def main():

    ## Set the scrape url
    avl_url = "https://wiki.teltonika-mobility.com/view/Full_AVL_ID_List"
    print(f"[AVL ID SCRAPER] Scrape url set to: {avl_url}")

    ## Get the web page content
    session = HTMLSession()
    r = session.get(avl_url)
    print(f"[AVL ID SCRAPPER] Request return code: {r.status_code}")

    ## validate the content and response
    if r.status_code != 200:
        print(f"[AVL ID SCRAPER] [ERROR] Error getting page. Response Code: {r.status_code}")
        exit(1)
    
    if "Multiplier".lower() not in r.text.lower():
        print(f"[AVL ID SCRAPER] [ERROR] Page content invalid. Content: \n\n{r.text.lower()[:2500]}")
        exit(1)
    
    ## Extract the tabular data and the headings
    # Headings css :: h2 span.mw-headline, h3 span.mw-headline

    tables = r.html.find("table.nd-othertables_2", first=False)
    print(f"[AVL ID SCRAPER] Found {len(tables)} Tables...")

    data = {}
    for table in tables:
        rows = table.find('tr')[2:]
        
        for row in rows:
            cells = row.find('td')

            property_id = cells[0].text.strip()
            property_name = cells[1].text.strip()
            multiplier = cells[6].text.strip()
            units = cells[7].text.strip()
            parameter_group = cells[10].text.strip()

            try:
                description_raw = cells[8].find('p', first=True).text.strip()
                description_list = description_raw.split('\n')
                description_dict = {}

                for item in description_list:
                    key, value = re.split("-|\u2013", item)
                    description_dict[int(key)] = value

            except Exception as e:
                print(f"[AVL ID SCRAPER] [PID: {property_id}] Warning: Split operation failed with error: {e}")
                description_dict = {'data': cells[8].text.strip()}

            data[property_id] = {
                'Property Name': property_name,
                'Multiplier': multiplier,
                'Units': units,
                'Description': description_dict,
                'Parameter Group': parameter_group
            }

    with open('avl_ids.json', 'w') as f:
        json.dump(data, f, indent=2, sort_keys=True)

    print(f"[AVL ID SCRAPER] Found {len(data)} IDs stored in alv_ids.json. Exiting.")





if __name__ == "__main__":
    main()
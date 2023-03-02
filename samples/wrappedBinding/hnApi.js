// This function will be called to wrap inner bindings passed as env argument.
// The return value is used as a resulting binding value.
export function wrapBindings(env) {
  return {
    async query(q) {
      let url = "http://hn.algolia.com/api/v1/search_by_date?";
      if (q.tags) {
        url += `tags=(${q.tags.join(",")})&`;
      }
      // env.internet is configured as inner binding.
      const response = await env.internet.fetch(new Request(url));
      return await response.json();
    }
  }
}
